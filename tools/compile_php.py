#!/usr/bin/python

# compile_php.py
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
import re
import hashlib


class PageParser:
    page_name = None
    page_text = None
    pos = None
    code = None
    next_block_id = None
    blocks = None

    def __init__(self, page_name, page_text):
        self.page_name = page_name
        self.page_text = page_text

    def parse(self):
        self.pos = 0
        self.code = []
        self.next_block_id = 0
        self.blocks = []

        try:
            self.parse_page()
        except Exception as e:
            lines = self.page_text[:self.pos].split('\n')
            if len(lines) == 0:
                lines.append('')

            raise RuntimeError("Error parsing '{0}' at line {1}, char {2}: {3}".format(self.page_name,
                                                                                       len(lines),
                                                                                       1 + len(lines[-1]),
                                                                                       e))

        return self.code

    def parse_page(self):
        self.code.append(['LABEL', '{0}_entry'.format(self.page_name)])

        while True:
            self.skip_whitespace()
            if self.eof():
                break

            if not (self.parse_open_tag() or self.parse_php_code() or self.parse_page_text() or self.parse_close_tag()):
                raise RuntimeError("Expected: open-tag, close-tag, page-text or php-code")

        self.code.append(['END'])

    def parse_open_tag(self):
        self.skip_whitespace()

        match = self.consume_regex(r'<([?]?[\w-]+)\s*')
        if not match:
            return False

        self.code.append(['OPEN_TAG', match.group(1)])

        is_pi = match.group(1)[0] == '?'
        end_regex = r'\s*([?]>)' if is_pi else r'\s*(/?>)'

        while True:
            end_match = self.consume_regex(end_regex)
            if end_match:
                if end_match.group(1)[0] == '/':
                    self.code.append(['CLOSE_TAG'])
                break
            if not (self.parse_attr() or self.parse_php_code()):
                raise RuntimeError("Expected: attr, php-code, or tag-end")

        return True

    def parse_attr(self):
        self.skip_whitespace()

        match = self.consume_regex(r'([\w-]+)="')
        if not match:
            return False

        self.code.append(['ATTR', match.group(1)])

        while not self.consume_regex(r'"'):
            if not (self.parse_attr_text() or self.parse_php_code()):
                raise RuntimeError('Expected: attr-text, php-code, or "')

        return True

    def parse_attr_text(self):
        match = self.consume_regex(r'[^"<]+')
        if not match:
            return False

        self.code.append(['TEXT', match.group(0)])

        return True

    def parse_php_code(self):
        self.skip_whitespace()

        match = self.consume_regex(r'<!--(.*?)-->')
        if not match:
            return False

        code = match.group(1).strip()
        if code == '':
            return True

        if self.parse_code_as_format_inst(code) or \
           self.parse_code_as_routine(code) or \
           self.parse_code_as_if_while(code) or \
           self.parse_code_as_end_block(code) or \
           self.parse_code_as_abort(code) or \
           self.parse_code_as_else(code) or \
           self.parse_code_as_nl(code):
            return True

        raise RuntimeError("Invalid PHP code: '{0}'".format(code))

    def parse_code_as_format_inst(self, code):
        match = re.match(r'=([^:]+):(\w+)$', code, re.S | re.I)
        if not match:
            return False

        self.code.append(['FORMAT', match.group(1), match.group(2)])

        return True

    def parse_code_as_routine(self, code):
        if not (code[0] == '{' and code[-1] == '}'):
            return False

        self.code.append(['ROUTINE', code[1:-1].strip() + ' return TRUE;'])

        return True

    def parse_code_as_if_while(self, code):
        match = re.match(r'(if|while)\s*(.*)$', code, re.S | re.I)
        if not match:
            return False

        block_name = match.group(1)
        cond_code = match.group(2).strip()

        if block_name == 'while':
            self.code.append(['LABEL', '{0}_while_{1}_start'.format(self.page_name, self.next_block_id)])

        self.code.append(['ROUTINE', 'return !(' + cond_code + ');'])
        self.code.append(['JUMP', '{0}_{1}_{2}_end'.format(self.page_name, block_name, self.next_block_id)])

        self.blocks.append([block_name, self.next_block_id, False])

        self.next_block_id += 1

        return True

    def parse_code_as_end_block(self, code):
        match = re.match(r'end\s*(if|while)$', code, re.S | re.I)
        if not match:
            return False

        block_name = match.group(1)

        if len(self.blocks) == 0 or self.blocks[-1][0] != block_name:
            raise RuntimeError("end{0} without corresponding {0}".format(block_name))

        _, block_id, has_else = self.blocks.pop()

        if block_name == 'while':
            self.code.append(['JUMP', '{0}_while_{1}_start'.format(self.page_name, block_id)])
        if not (block_name == 'if' and has_else):
            self.code.append(['LABEL', '{0}_{1}_{2}_end'.format(self.page_name, block_name, block_id)])
        else:
            self.code.append(['LABEL', '{0}_{1}_{2}_end2'.format(self.page_name, block_name, block_id)])

        return True

    def parse_code_as_abort(self, code):
        if code != 'abort':
            return False

        self.code.append(['END'])

        return True

    def parse_code_as_nl(self, code):
        if code != 'nl':
            return False

        self.code.append(['TEXT', "\n"])

        return True

    def parse_code_as_else(self, code):
        if code != 'else':
            return False

        if len(self.blocks) == 0 or self.blocks[-1][0] != 'if':
            raise RuntimeError("else without corresponding if")

        _, block_id, has_else = self.blocks[-1]

        if has_else:
            raise RuntimeError("Duplicate else in if")

        self.blocks[-1][2] = True

        self.code.append(['JUMP', '{0}_if_{1}_end2'.format(self.page_name, block_id)])
        self.code.append(['LABEL', '{0}_if_{1}_end'.format(self.page_name, block_id)])

        return True

    def parse_close_tag(self):
        self.skip_whitespace()

        match = self.consume_regex(r'</\s*([\w-]+)\s*>')
        if not match:
            return False

        self.code.append(['CLOSE_TAG'])

        return True

    def parse_page_text(self):
        self.skip_whitespace()

        match = self.consume_regex(r'[^<]+')
        if not match:
            return False

        self.code.append(['TEXT', match.group(0)])

        return True

    def eof(self):
        return self.pos >= len(self.page_text)

    def skip_whitespace(self):
        self.consume_regex(r'\s*')

    def consume_regex(self, pattern, flags=re.S | re.I):
        match = re.match(pattern, self.page_text[self.pos:], flags)
        if match:
            self.pos += match.end()

        return match


def scan_www_dir(www_dir):
    if www_dir[-1] == '/':
        www_dir = www_dir[:-1]

    q = collections.deque([www_dir])
    files = []

    while len(q) > 0:
        item = q.popleft()

        if os.path.isdir(item):
            q.extend([item + '/' + subdir for subdir in os.listdir(item)])
        elif item.endswith('.php'):
            files.append(item[len(www_dir) + 1:])

    return files


def load_page(www_dir, filename):
    with file(os.path.join(www_dir, filename), "r") as f:
        page_text = f.read()

    code = PageParser(filename, page_text).parse()

    return {
        'name': filename,
        'code': code
    }


def add_define_names(index, item_name, field='name'):
    names = []

    for i, key in enumerate(sorted(index.keys())):
        data = index[key]

        if isinstance(field, basestring):
            name = data[field]
        else:
            name = field(data)

        name = re.sub(r'[^a-z0-9]', '_', name, flags=re.I).upper()

        if name == '':
            name = 'NONE'

        full_name = name
        disc = 1
        while full_name in names:
            disc += 1
            full_name = name + str(disc)

        names.append(full_name)

        data['define_name'] = 'PHP_' + item_name + '_' + full_name


def join_code(pages):
    code = []

    for key in sorted(pages.keys()):
        page = pages[key]

        code.extend(page['code'])

    return code


def is_non_relocatable_label(instr):
    return instr[0] == 'LABEL' and (instr[1].endswith('_entry') or instr[1].startswith('sub_'))


def iter_self_contained_blocks(code, min_size=1):
    for start in xrange(len(code)):
        open_labels = set()
        rename_labels = dict()
        next_id = 1
        crc = hashlib.md5()

        for end in xrange(start, len(code)):
            instr = code[end]

            # Entry and exit points can never be part of a self-contained block
            if instr[0] == 'END' or is_non_relocatable_label(instr):
                break

            # Note: we rely on the fact that individual labels and jumps correspond 1:1
            if instr[0] in ['LABEL', 'JUMP']:
                label = instr[1]
                if label not in rename_labels:
                    rename_labels[label] = next_id
                    next_id += 1

                if label in open_labels:
                    open_labels.remove(label)
                else:
                    open_labels.add(label)

            instr_copy = list(instr)
            if instr[0] in ['LABEL', 'JUMP']:
                instr_copy[1] = rename_labels[instr_copy[1]]
            crc.update(repr(instr_copy))

            if len(open_labels) == 0 and ((1 + end - start) >= min_size):
                yield {
                    'start': start,
                    'end': end,
                    'digest': crc.digest()
                }


def iter_factorings(code):
    by_digest = collections.defaultdict(list)

    for block in iter_self_contained_blocks(code, min_size=4):
        by_digest[block['digest']].append(block)

    for blocks in by_digest.values():
        if len(blocks) <= 1:
            continue

        out_blocks = list()

        for block in sorted(blocks, key=lambda b: b['start']):
            if len(out_blocks) == 0 or block['start'] > out_blocks[-1]['end']:
                out_blocks.append(block)

        yield blocks


def apply_factoring(code, factoring):
    offset = 0

    num_subs = len([1 for instr in code if instr[0] == 'LABEL' and instr[1].startswith('sub_')])
    sub_name = 'sub_{0}'.format(num_subs + 1)

    saved_code = code[factoring[0]['start']:(factoring[0]['end'] + 1)]

    for block in factoring:
        code[(block['start'] - offset):(block['end'] + 1 - offset)] = [['CALL', sub_name]]
        offset += block['end'] - block['start']

    code.append(['LABEL', sub_name])
    code.extend(saved_code)
    code.append(['RET'])


def optimize_code(code):
    def factoring_score(factoring):
        return (1 + factoring[0]['end'] - factoring[0]['start']) * len(factoring)

    while True:
        best_factoring = None
        for factoring in iter_factorings(code):
            if best_factoring is None or factoring_score(factoring) > factoring_score(best_factoring):
                best_factoring = factoring

        if best_factoring is None:
            break

        apply_factoring(code, best_factoring)


def instruction_size(instr):
        if instr[0] == 'LABEL':
            return 0
        elif instr[0] in ['FORMAT', 'CALL', 'TEXT', 'JUMP']:
            return 2
        else:
            return 1


def index_nodes(code):
    nodes = {'': {'name': ''}}

    for instr in code:
        if instr[0] == 'OPEN_TAG':
            nodes[instr[1]] = {'name': instr[1]}

    add_define_names(nodes, 'NODE', field=lambda data: data['name'].replace('?', 'PI_'))

    for instr in code:
        if instr[0] == 'OPEN_TAG':
            instr[1] = nodes[instr[1]]['define_name']

    return nodes


def index_attrs(code):
    attrs = {'': {'name': ''}}

    for instr in code:
        if instr[0] == 'ATTR':
            attrs[instr[1]] = {'name': instr[1]}

    add_define_names(attrs, 'ATTR')

    for instr in code:
        if instr[0] == 'ATTR':
            instr[1] = attrs[instr[1]]['define_name']

    return attrs


def index_strings(code):
    strings = {'': {'text': ''}}

    for instr in code:
        if instr[0] == 'TEXT':
            strings[instr[1]] = {'text': instr[1]}

    add_define_names(strings, 'STR', field=lambda data: data['text'][:16])

    for instr in code:
        if instr[0] == 'TEXT':
            instr[1] = strings[instr[1]]['define_name']

    return strings


def index_vars(code):
    def adjust_var_name(name):
        if name.startswith('php.'):
            name = name[4:]
        if name.startswith('vars.'):
            name = name[5:]
        if name.startswith('entry.'):
            name = name[6:]

        name = name.replace('threshold', 'thresh')

        return name

    variables = {}

    for instr in code:
        if instr[0] == 'FORMAT':
            variables[instr[1]] = {'name': instr[1]}

    add_define_names(variables, 'VAR', field=lambda data: adjust_var_name(data['name']))

    for instr in code:
        if instr[0] == 'FORMAT':
            instr[1] = variables[instr[1]]['define_name']

    return variables


def index_conversions(code):
    conversions = {}

    for instr in code:
        if instr[0] == 'FORMAT':
            conversions[instr[2]] = {'name': instr[2]}

    add_define_names(conversions, 'CONV')

    for instr in code:
        if instr[0] == 'FORMAT':
            instr[2] = conversions[instr[2]]['define_name']

    return conversions


def index_routines(code):
    routines = {}

    routine_id = 0

    for instr in code:
        if instr[0] == 'ROUTINE':
            if not instr[1] in routines:
                name = str(routine_id)

                fn_name = '_php_routine_' + re.sub(r'[^0-9a-z_]', '_', name.lower(), flags=re.S | re.I)

                routines[instr[1]] = {'name': name,
                                      'fn_name': fn_name,
                                      'code': instr[1]}
                routine_id += 1

    add_define_names(routines, 'ROUTINE')

    for instr in code:
        if instr[0] == 'ROUTINE':
            instr[1] = routines[instr[1]]['define_name']

    return routines


def index_labels(code):
    labels = dict()

    offset = 0
    for instr in code:
        if instr[0] == 'LABEL':
            labels[instr[1]] = {
                'name': instr[1],
                'offset': offset
            }

        offset += instruction_size(instr)

    add_define_names(labels, 'LABEL')

    for instr in code:
        if instr[0] in ['JUMP', 'CALL']:
            instr[1] = labels[instr[1]]['define_name']

    return labels


def gen_header(pages):
    lines = list()

    def define(symbol, value):
        lines.append("#define {0:40} {1}".format(symbol, value))

    def nl():
        lines.append('')

    def gen_index(index, set_name, datatype='char', field='name', no_defines=False, limit=None):
        if limit is not None and len(index) > limit:
            raise RuntimeError("Number of {0} exceeds limit {1}".format(set_name.lower(), limit))

        nl()
        lines.append('static {0} const PHP_{1}[] PROGMEM ={2}'.format(datatype, set_name,
                                                                      '' if datatype == 'char' else ' {'))

        for i, key in enumerate(sorted(index.keys())):
            data = index[key]
            is_last = (i == len(index) - 1)

            if isinstance(field, basestring):
                value = data[field]
            else:
                value = field(data)

            if datatype == 'char':
                escaped_str = value.replace("\n", "\\n").replace("\t", "\\t").replace('"', '\\"')
                lines.append('    "{0}\\0"{1}'.format(escaped_str, ';' if is_last else ''))
            else:
                lines.append('    ({0}){1}{2}'.format(datatype, value, '' if is_last else ','))

        if datatype != 'char':
            lines.append('};')

        if not no_defines:
            nl()
            for i, key in enumerate(sorted(index.keys())):
                define(index[key]['define_name'], i)

    code = join_code(pages)

    optimize_code(code)

    nodes = index_nodes(code)
    attrs = index_attrs(code)
    strings = index_strings(code)
    variables = index_vars(code)
    conversions = index_conversions(code)
    routines = index_routines(code)
    labels = index_labels(code)

    add_define_names(pages, 'PAGE')

    lines.extend([
        '/*',
        ' * php_code.h - Compiled "PHP" code',
        ' *',
        ' * (C) Copyright 2009-2013  Cristian Dinu <goc9000@gmail.com>',
        ' *',
        ' * Licensed under the GPL-3.',
        ' *',
        ' * THIS FILE IS AUTOGENERATED. DO NOT EDIT MANUALLY!',
        ' */',
    ])
    nl()
    define('PHP_PAGES_COUNT', len(pages))
    gen_index(index=pages, set_name='PAGES')
    gen_index(index=pages, set_name='PAGES_ENTRY', datatype='uint16_t', no_defines=True,
              field=lambda page: labels[page['name']+'_entry']['offset'])
    gen_index(index=nodes, set_name='NODES', limit=64)
    gen_index(index=attrs, set_name='ATTRS', limit=64)
    gen_index(index=strings, set_name='STRINGS', field='text', limit=256)
    gen_index(index=variables, set_name='VARS', datatype='void *', limit=256,
              field=lambda data: '&' + data['name'])
    gen_index(index=conversions, set_name='CONVERSIONS', datatype='php_conv_fn_t', limit=32,
              field=lambda data: '_php_format_' + data['name'])

    for routine in sorted(routines.values(), key=lambda r: r['code']):
        nl()
        lines.extend([
            'static bool {0}(void) {{'.format(routine['fn_name']),
            '    {0}'.format(routine['code']),
            '}'
        ])

    gen_index(index=routines, set_name='ROUTINES', datatype='php_routine_fn_t', limit=64,
              field='fn_name')

    nl()
    for label in sorted(labels.values(), key=lambda l: l['offset']):
        define(label['define_name'], label['offset'])
    nl()
    lines.append('static const uint8_t PHP_CODE[] PROGMEM = {')
    for i, instr in enumerate(code):
        if instr[0] == 'LABEL':
            lines.append('    // ' + instr[1])
            continue
        args = ('(' + ','.join(instr[1:]) + ')') if len(instr) > 1 else ''
        lines.append('    PHP_UCODE_{0}{1}{2}'.format(instr[0], args, '' if i == len(code) - 1 else ','))
    lines.append('};')
    nl()

    return '\n'.join(lines)


parser = argparse.ArgumentParser()
parser.add_argument('www_dir', metavar='<wwwroot directory>')
parser.add_argument('out_file', metavar='<output header file>')
args = parser.parse_args()

try:
    if not os.path.isdir(args.www_dir):
        raise RuntimeError("Cannot open wwwroot directory '{0}'".format(args.www_dir))

    files = scan_www_dir(args.www_dir)

    pages = dict((filename, load_page(args.www_dir, filename)) for filename in sorted(files))

    with file(args.out_file, "w") as f:
        f.write(gen_header(pages))
except Exception as exc:
    print >> sys.stderr, "Error:", exc
