import os
import sys
import buildimpl
import buildutil

def task_all():
    return {
        'actions': [],
        'task_dep': ['build', 'eclipse', 'create_release'],
        'doc': ': Do everything!',
        'verbosity': 2
    }

def task_build():
    return {
        'task_dep': ['build_cpp', 'build_jvm'],
        'actions': [],
        'doc': ': Build GPT',
        'verbosity': 2
    }

def task_build_cpp():
    return {
        'task_dep': ['generate'],
        'file_dep': cpp_files,
        'actions': [buildimpl.build_cpp],
        'targets': ['gpt-displaystransmitter/target'],
        'clean': [buildimpl.clean_cpp],
        'doc': ': Build GPT c++ parts',
        'verbosity': 2
    }

def task_build_jvm():
    return {
        'task_dep': ['generate'],
        'file_dep': jvm_files,
        'actions': [buildimpl.build_jvm],
        'targets': ["gpt-common/target"],
        'clean': [buildimpl.clean_jvm],
        'doc': ': Build GPT jvm parts',
        'verbosity': 2
    }
  
def task_eclipse():
    return {
        'file_dep': jvm_files,
        'task_dep': ['generate'],
        'actions': [buildimpl.eclipse],
        'doc': ': Create eclipse projects',
        'verbosity': 2
    }
    
def task_generate():
    return {
        'file_dep': model_files,
        'actions': [buildimpl.generate_model, updateGeneratedFileLists],
        'clean': [buildimpl.clean_gen],
        'targets': ["gpt-common/src_generated"],
        'doc': ': Generate cross-language data model source code for GPT',
        'verbosity': 2
    }
    
def task_create_release():
    return {
        'file_dep': all_input_files,
        'task_dep': ['build'],
        'actions': [buildimpl.release],
        'clean': [buildimpl.clean_release],
        'targets': ["release/"],
        'doc': ': Create a new release zip of GPT',
        'verbosity': 2
    }
    
###############################################################################
###############################################################################

# Helpers for generated code changes

model_files = buildutil.findFilesExt('gpt-common/models', '.xml')
jvm_files_manual =  buildutil.findFiles('gpt-common/models', '*.xml') + \
                    buildutil.findFilesExt('gpt-common/src', ['.java', '.scala', '.sbt']) + \
                    buildutil.findFilesExt('gpt-displaysreceiver/src', ['.java', '.scala', '.sbt']) + \
                    buildutil.findFilesExt('gpt-keyreceiver/src', ['.java', '.scala', '.sbt']) + \
                    buildutil.findFilesExt('gpt-keytransmitter/src', ['.java', '.scala', '.sbt']) + \
                    buildutil.findFilesExt('gpt-shmreceiver/src', ['.java', '.scala', '.sbt']) + \
                    buildutil.findFilesExt('gpt-shmtransmitter/src', ['.java', '.scala', '.sbt'])
cpp_files_manual =  buildutil.findFiles('gpt-common/models', '*.xml') + \
                    buildutil.findFilesExt('gpt-displaysreceiver/src', ['.cpp', '.h', 'CMakeLists.txt'])

jvm_files = []
cpp_files = []
all_input_files = []
            
def updateGeneratedFileLists():
    del jvm_files[:]
    del cpp_files[:]
    del all_input_files[:]
    jvm_files.extend(jvm_files_manual + buildutil.findFilesExt('gpt-common/src_generated', ['.java', '.scala', '.sbt']))
    cpp_files.extend(cpp_files_manual + buildutil.findFilesExt('gpt-common/src_generated', ['.cpp', '.h']))
    all_input_files.extend(jvm_files + cpp_files)
    
updateGeneratedFileLists()

