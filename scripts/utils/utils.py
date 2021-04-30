# Copyright (c) 2021 Valve Corporation
# Copyright (c) 2021 LunarG, Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Nathaniel Cesario <nathaniel@lunarg.com>

# Various utility functions/classes

import os, shutil
import urllib.parse
from urllib.parse import urlparse
import http.client as http

def make_or_exist_dirs(path, clean=False):
    "Wrapper for os.makedirs that tolerates the directory already existing"
    # Could use os.makedirs(path, exist_ok=True) if we drop python2
    if not os.path.isdir(path):
        os.makedirs(path)
    elif clean:
        shutil.rmtree(path)
        os.makedirs(path)

# Utility for making simple GET requests
# Usage: with URLRequest('https://example.com/path/to/file.txt') as res: print(res.read().decode('utf-8'))
# NOTE: May make more sense to just use the requests package (https://pypi.org/project/requests/)
class URLRequest:
    def __init__(self, url):
        self.url = None
        if type(url) == str: self.url = urlparse(url.strip())
        elif type(url) == urllib.parse.ParseResult: self.url = url
        else: raise Exception(f'Unknown URL type: {type(url)}')

        self.conn_ = None
        self.connect()

        res = self.conn_.getresponse()
        while res.status == 302:
            # redirect
            url = None
            for h in res.getheaders():
                if h[0] == 'Location': url = h[1]
            if url is None: raise Exception('Encountered redirect during request, but did not find redirect location')

            self.conn_.close()
            self.url = urlparse(url)
            self.connect()
            res = self.conn_.getresponse()
        self.res = res

    def __enter__(self): return self.res
    def __exit__(self, type, value, tb):
        self.conn_.close()
        return False

    def connect(self):
        if self.url.scheme == 'http': self.conn_ = http.HTTPConnection(self.url.netloc)
        elif self.url.scheme == 'https': self.conn_ = http.HTTPSConnection(self.url.netloc)
        else: raise Exception(f'Unknown scheme {self.url.scheme}')
        self.conn_.request('GET', f'{self.url.path}?{self.url.query}')

    def close(self): self.conn_.close()

# Currently only gzip tar and zip files are supported
def expand_archive(path):
    if path.endswith('tar.gz') or path.endswith('tgz'):
        import tarfile
        with tarfile.open(path, 'r:gz') as tar: tar.extractall()
    elif path.endswith('.zip'):
        import zipfile
        with zipfile.ZipFile(path, 'r') as archive: archive.extractall('.')
    else:
        raise Exception(f'Could not expand archive at {path}')
