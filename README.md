### C-Low level library

Personal use low level, since it's C, primarly an allocator library (for now).
The aim of the project is to collect many independant and portable C utilities where third-parties are not well suited or do not have a good API, to be used in realtime/fast and clean APIs for game dev.

Plans in the future to add more utilities custom stuff.

Written in ANSI C, compliant to all C version up to C17 (at least what I've tested), but also compiles for C++.


### Features

- `freelist` Basically a non fixed size slab allocator with internal linked list tracking of free memory.
- `gpalloc` General purpose allocator with external linked list tracking of free memory with alignment in mind.

### Usage

Clone/download the repo add `include` to your project or just copy the needed files.

See tests.

### License

LICENSE: BSD-2
Copyright (c) 2025, Kirichenko Stanislav
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions, and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions, and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.


### Credits

Not necessary, but greatly appreciated. If you use it in your project we can link your project in the readme if you want.
Contact me through a pull request/issue.


### Contribution

If you want to submit you own utility, one rule is to have not depend on external, non standard stuff.
Compile in C but also C++, be as portable/crossplatform as possible. And make automatic tests for it.
