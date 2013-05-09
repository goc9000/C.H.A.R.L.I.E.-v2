#!/usr/bin/python

# mkrawfsimg.py
#
# (C) Copyright 2013  Cristian Dinu <goc9000@gmail.com>
#
# This file is part of charliev2.
#
# Licensed under the GPL-3

import argparse
import collections
import os
import sys
import struct


BLOCK_SIZE = 512
INDEX_BLOCKS = 32
DATA_FILE_BLOCKS = 131072


def scan_contents_dir(contents_dir):
    if contents_dir[-1] == '/':
        contents_dir = contents_dir[:-1]

    q = collections.deque([contents_dir])
    files = []

    while len(q) > 0:
        item = q.popleft()

        if os.path.isdir(item):
            q.extend([item + '/' + subdir for subdir in os.listdir(item)])
        else:
            files.append(item[len(contents_dir) + 1:])

    return files


def create_index(contents_dir, files):
    index = []

    base_addr = BLOCK_SIZE * INDEX_BLOCKS

    for filename in sorted(files, key=lambda item: (chr(255) if item.startswith('data/') else '') + item):
        if filename.startswith('data/'):
            file_length = 0
            capacity = BLOCK_SIZE * DATA_FILE_BLOCKS
        else:
            file_length = os.path.getsize(os.path.join(contents_dir, filename))
            capacity = BLOCK_SIZE * max(1, ((file_length + BLOCK_SIZE - 1) // BLOCK_SIZE))

        item = {
            'filename': filename,
            'address': base_addr,
            'length': file_length,
            'capacity': capacity
        }

        index.append(item)

        base_addr += capacity

    return index


def write_index(out_file, index):
    for item in index:
        out_file.write(struct.pack('<52sIII', item['filename'], item['address'], item['length'], item['capacity']))

    out_file.write('\0' * (BLOCK_SIZE * INDEX_BLOCKS - 64 * len(index)))


def write_files(out_file, contents_dir, index):
    for item in index:
        if not item['filename'].startswith('data/'):
            with file(os.path.join(contents_dir, item['filename']), 'rb') as f:
                out_file.write(f.read())
                out_file.write('\0' * (item['capacity'] - item['length']))


parser = argparse.ArgumentParser()
parser.add_argument('contents_dir', metavar='<contents directory>')
parser.add_argument('out_image_file', metavar='<output image file>')
args = parser.parse_args()

try:
    if not os.path.isdir(args.contents_dir):
        raise RuntimeError("Cannot open contents directory '{0}'".format(args.contents_dir))

    files = scan_contents_dir(args.contents_dir)
    index = create_index(args.contents_dir, files)

    with file(args.out_image_file, "wb") as f:
        write_index(f, index)
        write_files(f, args.contents_dir, index)

except Exception as exc:
    print >> sys.stderr, "Error:", exc
