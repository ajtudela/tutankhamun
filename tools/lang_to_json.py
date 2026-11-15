#!/usr/bin/env python3
"""Turns a translated edition's text files into a language pack of the game: config/lang/<code>.json.

The sources are the edition's own files, keyed as the game's: the text file and the message file either as the game
has them (Pharaoh_Text.eng, Pharaoh_MM.eng) or as engconverter's XML of them (<strings>/<group>/<string>,
<messages>/<message>/<title>/<subtitle>/<content>), and eventmsg.txt (KEY "text" lines). The pack holds the texts as
UTF-8 by group and index, the messages by their index in the message file and the event messages by key; the game
lays them over its English files (src/game/text/lang_pack.c).

    python tools/lang_to_json.py --code de --codepage cp1252 --text Pharaoh_Text.eng --mm Pharaoh_MM.eng \
        --source "German edition 2.0" config/lang/de.json

The text and the event files are in the edition's codepage (--codepage). An XML that was written by reading such a
file as cp1252 (the Polish one) is turned back with --xml-codepage cp1252 before it is read in the edition's codepage.
"""

import argparse
import json
import re
import struct
import sys
import xml.etree.ElementTree as ET

TEXT_HEADER_SIZE = 28
TEXT_GROUPS = 1000
MESSAGE_HEADER_SIZE = 24
MESSAGE_COUNT = 1000
MESSAGE_RECORD_SIZE = 80
MESSAGE_TEXT_OFFSETS = 0x44  # title, subtitle, content (lang_message in src/game/text/lang_message_types.h)


def text_from_eng(path, codepage):
    data = open(path, 'rb').read()
    index = [struct.unpack_from('<ii', data, TEXT_HEADER_SIZE + 8 * g) for g in range(TEXT_GROUPS)]
    base = data[TEXT_HEADER_SIZE + 8 * TEXT_GROUPS:]
    starts = sorted({offset for offset, in_use in index if in_use})
    groups = {}
    for group, (offset, in_use) in enumerate(index):
        if not in_use:
            continue
        end = min([s for s in starts if s > offset] + [len(base)])
        strings, current, previous = [], bytearray(), 0
        # the strings of lang_get_string: a zero after a printable byte or after another zero ends one
        for byte in base[offset:end]:
            if byte == 0 and (previous >= 0x20 or previous == 0):
                strings.append(bytes(current).lstrip(bytes(range(0x20))).decode(codepage))
                current = bytearray()
            else:
                current.append(byte)
            previous = byte
        groups[group] = strings
    return groups


def turn_back(text, xml_codepage, codepage):
    if not xml_codepage:
        return text
    return text.encode(xml_codepage, errors='replace').decode(codepage, errors='replace')


def text_from_xml(path, xml_codepage, codepage):
    groups = {}
    for group in ET.parse(path).getroot().iter('group'):
        strings = {}
        for string in group.iter('string'):
            strings[int(string.get('id'))] = turn_back(string.text or '', xml_codepage, codepage)
        count = max(strings) + 1 if strings else 0
        groups[int(group.get('id'))] = [strings.get(i, '') for i in range(count)]
    return groups


def messages_from_eng(path, codepage):
    data = open(path, 'rb').read()
    texts = data[MESSAGE_HEADER_SIZE + MESSAGE_RECORD_SIZE * MESSAGE_COUNT:]

    def at(offset):
        return texts[offset:texts.index(b'\0', offset)].decode(codepage)

    messages = {}
    for m in range(MESSAGE_COUNT):
        record = MESSAGE_HEADER_SIZE + MESSAGE_RECORD_SIZE * m
        title, subtitle, content = struct.unpack_from('<iii', data, record + MESSAGE_TEXT_OFFSETS)
        if not (title or subtitle or content):
            continue
        message = {}
        for field, offset in (('title', title), ('subtitle', subtitle), ('content', content)):
            if offset:
                message[field] = at(offset)
        messages[m] = message
    return messages


def messages_from_xml(path, xml_codepage, codepage):
    messages = {}
    for node in ET.parse(path).getroot().iter('message'):
        message = {}
        for field in ('title', 'subtitle', 'content'):
            element = node.find(field)
            if element is not None and element.text:
                message[field] = turn_back(element.text, xml_codepage, codepage)
        if message:
            messages[int(node.get('id'))] = message
    return messages


def events_from_txt(path, codepage):
    text = open(path, 'rb').read().decode(codepage)
    events = {}
    # KEY "text": a comment line starts with ';', a backslash at the end of a line goes on with the next one
    for match in re.finditer(r'^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]+"((?:[^"\\]|\\[^\r\n]|\\\r?\n[ \t]*)*)"', text, re.M):
        events[match.group(1)] = re.sub(r'\\\r?\n[ \t]*', '', match.group(2))
    return events


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--code', required=True)
    parser.add_argument('--codepage', default='cp1252')
    parser.add_argument('--xml-codepage', default='')
    parser.add_argument('--text', required=True, help='Pharaoh_Text.eng or its XML')
    parser.add_argument('--mm', help='Pharaoh_MM.eng or its XML')
    parser.add_argument('--events', help='eventmsg.txt')
    parser.add_argument('--source', required=True, help='where the files come from, kept in the pack')
    parser.add_argument('out')
    args = parser.parse_args()

    def is_xml(path):
        return path.lower().endswith('.xml')

    texts = (text_from_xml(args.text, args.xml_codepage, args.codepage) if is_xml(args.text)
             else text_from_eng(args.text, args.codepage))
    messages = {}
    if args.mm:
        messages = (messages_from_xml(args.mm, args.xml_codepage, args.codepage) if is_xml(args.mm)
                    else messages_from_eng(args.mm, args.codepage))
    events = events_from_txt(args.events, args.codepage) if args.events else {}

    pack = {
        'language': args.code,
        'source': args.source,
        'texts': {str(g): texts[g] for g in sorted(texts)},
        'messages': {str(m): messages[m] for m in sorted(messages)},
        'events': events,
    }
    with open(args.out, 'w', encoding='utf-8', newline='\n') as out:
        json.dump(pack, out, ensure_ascii=False, indent=1)
        out.write('\n')
    print(f'{args.out}: {sum(len(v) for v in texts.values())} texts in {len(texts)} groups, '
          f'{len(messages)} messages, {len(events)} event texts', file=sys.stderr)


if __name__ == '__main__':
    main()
