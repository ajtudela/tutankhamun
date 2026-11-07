// The game's activity: SDL's activity with the game in libmain.so (its main function), the gameplay configs copied from
// the APK's assets into the app's storage before the game starts, and the game folder picker that the game's thread
// asks for (showDirectorySelection) and waits on until gotDirectory. After Julius' JuliusMainActivity
// (https://github.com/bvschaik/julius, AGPL-3.0).

package com.github.dalerank.tutankhamun;

import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class TutankhamunActivity extends SDLActivity {
    private static final String TAG = "tutankhamun";
    private static final String CONFIG_ASSETS = "config";
    private static final int GET_FOLDER_RESULT = 500;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        copyConfigs();
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onStop() {
        super.onStop();
        FileManager.clearCache();
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{"SDL2", "SDL2_mixer", "main"};
    }

    @Override
    protected String getMainFunction() {
        return "main";
    }

    private void copyConfigs() {
        AssetManager assets = getAssets();
        File target = new File(getFilesDir(), CONFIG_ASSETS);
        if (!target.isDirectory() && !target.mkdirs()) {
            Log.e(TAG, "cannot make " + target);
            return;
        }
        byte[] buffer = new byte[65536];
        try {
            String[] names = assets.list(CONFIG_ASSETS);
            if (names == null) {
                return;
            }
            for (String name : names) {
                try (InputStream in = assets.open(CONFIG_ASSETS + "/" + name);
                     OutputStream out = new FileOutputStream(new File(target, name))) {
                    int read;
                    while ((read = in.read(buffer)) > 0) {
                        out.write(buffer, 0, read);
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "cannot copy the configs: " + e);
        }
    }

    @SuppressWarnings("unused")
    public void showDirectorySelection(boolean again) {
        startActivityForResult(DirectorySelectionActivity.newIntent(this, again), GET_FOLDER_RESULT);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == GET_FOLDER_RESULT) {
            if (resultCode == RESULT_OK && data != null && data.getData() != null) {
                FileManager.setBaseUri(data.getData());
            } else {
                FileManager.setBaseUri(Uri.EMPTY);
            }
            gotDirectory();
        } else {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    private native void gotDirectory();
}
