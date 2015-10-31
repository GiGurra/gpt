import os
import sys
import buildimpl
import buildutil

def task_all():
    return {
        'actions': [],
        'task_dep': ['build', 'create_release'],
        'doc': ': Do everything!',
        'verbosity': 2
    }

def task_build():
    return {
        'task_dep': ['build_jvm'],
        'actions': [],
        'doc': ': Build GPT',
        'verbosity': 2
    }

def task_build_jvm():
    return {
        'task_dep': ['generate'],
        'file_dep': jvm_build_file_dep,
        'actions': [buildimpl.build_jvm],
        'targets': ["gpt-common/target"],
        'clean': [buildimpl.clean_jvm],
        'doc': ': Build GPT jvm parts',
        'verbosity': 2
    }

def task_generate():
    return {
        'file_dep': codegen_model_file_dep,
        'actions': [buildimpl.generate_model],
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
###     Sources     ###
#######################

codegen_model_file_dep = buildutil.findFiles('gpt-common/models', '*.xml')

jvm_build_file_dep = \
    codegen_model_file_dep + \
    buildutil.findFilesExt('gpt-common/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-displaysreceiver/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-displaystransmitter/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-keyreceiver/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-keytransmitter/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-shmreceiver/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-shmtransmitter/src', ['.java', '.scala', '.sbt']) + \
    buildutil.findFilesExt('gpt-common/src_generated', ['.java', '.scala', '.sbt'])

all_input_files = jvm_build_file_dep
