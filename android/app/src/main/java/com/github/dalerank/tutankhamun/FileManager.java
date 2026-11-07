// The files of the game folder the user picked, through the Storage Access Framework, for the game's native code
// (src/platform/platform_android.c): a path of the game (relative to the folder, with \ or /, any case) is looked up
// part by part in the folder's documents, whose listings are cached; a file is opened as a file descriptor the native
// code owns, created when it is opened for writing and missing. After Julius' FileManager
// (https://github.com/bvschaik/julius, AGPL-3.0).

package com.github.dalerank.tutankhamun;

import android.app.Activity;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

public class FileManager {
    private static final String TAG = "tutankhamun";
    private static final int PATH_NONE = 0;
    private static final int PATH_FILE = 1;
    private static final int PATH_DIR = 2;
    private static final String MIME_BINARY = "application/octet-stream";

    private static Uri baseUri = Uri.EMPTY;
    private static final HashMap<Uri, HashMap<String, FileInfo>> directoryCache = new HashMap<>();

    @SuppressWarnings("unused")
    public static String getBaseUri() {
        return baseUri == Uri.EMPTY ? "" : baseUri.toString();
    }

    @SuppressWarnings("unused")
    public static boolean setBaseUri(String uri) {
        return uri != null && !uri.isEmpty() && setBaseUri(Uri.parse(uri));
    }

    static boolean setBaseUri(Uri uri) {
        directoryCache.clear();
        if (uri == null || uri == Uri.EMPTY) {
            baseUri = Uri.EMPTY;
            FileInfo.base = null;
            return false;
        }
        try {
            FileInfo.base = new FileInfo(DocumentsContract.getTreeDocumentId(uri), "", DocumentsContract.Document.MIME_TYPE_DIR, Uri.EMPTY);
            baseUri = uri;
            return true;
        } catch (Exception e) {
            Log.e(TAG, "setBaseUri: " + e);
            baseUri = Uri.EMPTY;
            FileInfo.base = null;
            return false;
        }
    }

    static void clearCache() {
        directoryCache.clear();
    }

    private static String key(String name) {
        return name.toLowerCase(Locale.ROOT);
    }

    private static String[] pathParts(String path) {
        ArrayList<String> parts = new ArrayList<>();
        for (String part : path.split("[\\\\/]")) {
            if (!part.isEmpty() && !part.equals(".")) {
                parts.add(part);
            }
        }
        return parts.toArray(new String[0]);
    }

    private static HashMap<String, FileInfo> listing(Activity activity, FileInfo dir) {
        Uri dirUri = dir.getUri();
        HashMap<String, FileInfo> result = directoryCache.get(dirUri);
        if (result != null) {
            return result;
        }
        result = new HashMap<>();
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(baseUri, dir.documentId);
        String[] columns = {
                DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE
        };
        try (Cursor cursor = activity.getContentResolver().query(children, columns, null, null, null)) {
            while (cursor != null && cursor.moveToNext()) {
                FileInfo info = new FileInfo(cursor.getString(0), cursor.getString(1), cursor.getString(2), dirUri);
                result.put(key(info.name), info);
            }
        } catch (Exception e) {
            Log.e(TAG, "listing " + dir.name + ": " + e);
        }
        directoryCache.put(dirUri, result);
        return result;
    }

    private static FileInfo find(Activity activity, String[] parts, int count) {
        if (FileInfo.base == null) {
            return null;
        }
        FileInfo current = FileInfo.base;
        for (int i = 0; i < count; i++) {
            if (!current.isDirectory()) {
                return null;
            }
            current = listing(activity, current).get(key(parts[i]));
            if (current == null) {
                return null;
            }
        }
        return current;
    }

    @SuppressWarnings("unused")
    public static int pathKind(Activity activity, String path) {
        String[] parts = pathParts(path);
        FileInfo info = find(activity, parts, parts.length);
        if (info == null) {
            return PATH_NONE;
        }
        return info.isDirectory() ? PATH_DIR : PATH_FILE;
    }

    @SuppressWarnings("unused")
    public static String[] listDirectory(Activity activity, String path) {
        String[] parts = pathParts(path);
        FileInfo dir = find(activity, parts, parts.length);
        if (dir == null || !dir.isDirectory()) {
            return new String[0];
        }
        ArrayList<String> names = new ArrayList<>();
        for (FileInfo info : listing(activity, dir).values()) {
            names.add(info.name);
        }
        return names.toArray(new String[0]);
    }

    private static FileInfo create(Activity activity, String path, String mimeType) {
        String[] parts = pathParts(path);
        if (parts.length == 0) {
            return null;
        }
        FileInfo parent = find(activity, parts, parts.length - 1);
        if (parent == null || !parent.isDirectory()) {
            return null;
        }
        String name = parts[parts.length - 1];
        try {
            Uri created = DocumentsContract.createDocument(activity.getContentResolver(), parent.getUri(), mimeType, name);
            if (created == null) {
                return null;
            }
            FileInfo info = new FileInfo(DocumentsContract.getDocumentId(created), name, mimeType, parent.getUri());
            listing(activity, parent).put(key(name), info);
            return info;
        } catch (Exception e) {
            Log.e(TAG, "create " + path + ": " + e);
            return null;
        }
    }

    @SuppressWarnings("unused")
    public static boolean createDirectory(Activity activity, String path) {
        String[] parts = pathParts(path);
        FileInfo existing = find(activity, parts, parts.length);
        if (existing != null) {
            return false;
        }
        return create(activity, path, DocumentsContract.Document.MIME_TYPE_DIR) != null;
    }

    @SuppressWarnings("unused")
    public static boolean deleteFile(Activity activity, String path) {
        String[] parts = pathParts(path);
        FileInfo info = find(activity, parts, parts.length);
        if (info == null || info == FileInfo.base) {
            return false;
        }
        try {
            HashMap<String, FileInfo> siblings = directoryCache.get(info.parent);
            if (siblings != null) {
                siblings.remove(key(info.name));
            }
            directoryCache.remove(info.getUri());
            return DocumentsContract.deleteDocument(activity.getContentResolver(), info.getUri());
        } catch (Exception e) {
            Log.e(TAG, "delete " + path + ": " + e);
            return false;
        }
    }

    @SuppressWarnings("unused")
    public static int openFileDescriptor(Activity activity, String path, String mode) {
        try {
            String[] parts = pathParts(path);
            FileInfo info = find(activity, parts, parts.length);
            boolean writes = mode.indexOf('w') >= 0;
            if (info == null && writes) {
                info = create(activity, path, MIME_BINARY);
            }
            if (info == null || info.isDirectory()) {
                return -1;
            }
            ParcelFileDescriptor descriptor = activity.getContentResolver().openFileDescriptor(info.getUri(), mode);
            return descriptor == null ? -1 : descriptor.detachFd();
        } catch (Exception e) {
            Log.e(TAG, "open " + path + " (" + mode + "): " + e);
            return -1;
        }
    }

    private static class FileInfo {
        static FileInfo base;
        final String documentId;
        final String name;
        final String mimeType;
        final Uri parent;
        private Uri uri = Uri.EMPTY;

        FileInfo(String documentId, String name, String mimeType, Uri parent) {
            this.documentId = documentId;
            this.name = name;
            this.mimeType = mimeType;
            this.parent = parent;
        }

        boolean isDirectory() {
            return DocumentsContract.Document.MIME_TYPE_DIR.equals(mimeType);
        }

        Uri getUri() {
            if (uri == Uri.EMPTY && baseUri != Uri.EMPTY) {
                uri = DocumentsContract.buildDocumentUriUsingTree(baseUri, documentId);
            }
            return uri;
        }
    }
}
