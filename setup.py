from setuptools import setup, Extension

sources = ['./aclpython/atcoder/atcoder.cpp']
extra_compile_args = ['-std=c++17', '-O2', '-mtune=native',
                      '-march=native', '-I./aclpython']
extensions = [Extension('atcoder',
                        sources=sources,
                        extra_compile_args=extra_compile_args)]

setup(name='atcoder', version='0.1', ext_modules=extensions)