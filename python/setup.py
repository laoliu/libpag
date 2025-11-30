import os
import sys
import subprocess
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        # 确保使用绝对路径，并为 CMake 设置正确的路径
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = [
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}',
            f'-DPYTHON_EXECUTABLE={sys.executable}',
            '-DPAG_BUILD_FRAMEWORK=OFF',
            '-DPAG_BUILD_SHARED=OFF',
            '-DPAG_USE_SWIFTSHADER=OFF',
        ]

        cfg = 'Release' if not self.debug else 'Debug'
        build_args = ['--config', cfg]

        cmake_args += [f'-DCMAKE_BUILD_TYPE={cfg}']
        
        # 设置并行编译
        cpu_count = os.cpu_count() or 4
        if sys.platform.startswith('darwin') or sys.platform.startswith('linux'):
            build_args += ['--', f'-j{cpu_count}']
        elif sys.platform == 'win32':
            build_args += ['--', f'/m:{cpu_count}']

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        env = os.environ.copy()
        
        # 运行 CMake
        print(f"Running CMake in {self.build_temp}")
        print(f"CMake args: {cmake_args}")
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        
        print(f"Building target: pypag")
        subprocess.check_call(['cmake', '--build', '.', '--target', 'pypag'] + build_args, cwd=self.build_temp, env=env)
        
        # 复制生成的文件到输出目录
        build_lib = os.path.join(self.build_temp, 'pypag*.so')
        import glob
        import shutil
        
        so_files = glob.glob(build_lib)
        if not so_files:
            raise RuntimeError(f"Could not find pypag module in {self.build_temp}")
        
        for src in so_files:
            # 重命名为标准的 Python 模块名
            dst = os.path.join(extdir, 'pypag' + os.path.splitext(src)[1])
            print(f"Copying {src} -> {dst}")
            shutil.copy(src, dst)
            break  # 只需要第一个


setup(
    name='pypag',
    version='0.1.0',
    author='PAG Team',
    author_email='',
    description='Python bindings for libpag',
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    url='https://github.com/Tencent/libpag',
    license='Apache-2.0',
    ext_modules=[CMakeExtension('pypag', sourcedir='..')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
    python_requires='>=3.7',
    install_requires=[
        'numpy>=1.19.0',
    ],
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Topic :: Multimedia :: Graphics',
        'License :: OSI Approved :: Apache Software License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
    ],
)
