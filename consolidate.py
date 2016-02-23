# -*- coding: utf-8 -*-
"""
Created on Mon Feb 22 12:04:11 2016

@author: cjacobs
"""

import os
    
def findFile(filename, searchDirectories):
    for d in searchDirectories:
        if os.path.isfile(os.path.join(d, filename)):
            return os.path.join(d, filename)

def readFile(filePath):
    if not filePath or not os.path.isfile(filePath):
        return None
    with open(filePath) as f:
        return f.readlines()
        
def addFile(fileDict, systemIncludes, filename, searchDirectories):    
    if filename not in fileDict:
        # search for filename in this directory or searchDirectories
        filePath = findFile(filename, searchDirectories)
        contents = readFile(filePath)
        thisFile = {'filename': filename, 'contents': contents, 'dependencies': []}        
        fileDict[filename] = thisFile
        if contents:
            for line in contents:
                line = line.strip()
                if line.startswith('#include'):
                    part = line[8:].strip()
                    if part.startswith('"'):
                        rpos = part[1:].find('"')
                        fname = part[1:rpos+1]
                        thisFile['dependencies'].append(fname)
                        addFile(fileDict, systemIncludes, fname, searchDirectories)
                    elif part.startswith('<'):
                        rpos = part[1:].find('>')
                        fname = part[1:rpos+1]
                        if fname not in systemIncludes:
                            systemIncludes.add(fname)
                    else:
                        print "Error in line: ", line
        
def findFirstFreeEntry(fileGraph):
    for g in fileGraph:
        if len(fileGraph[g]['dependencies']) == 0:
            return g
    return None

def removeDependency(fileGraph, dep):
    for nodeName in fileGraph:
        node = fileGraph[nodeName]
        if dep in node['dependencies']:
            node['dependencies'].remove(dep)
            
def consolidateSource(sourceFiles, includeDirectories, outfilename):
    # file obj: filename, set of dependencies, contents (if not system include)
    #  contents of system include = "#include <...>"
    fileGraph = {}
    systemIncludes = set()
    for sourceFile in sourceFiles:
        addFile(fileGraph, systemIncludes, sourceFile, ['.'] + includeDirectories)
    
    with open(outfilename, 'w') as outFile:
        # Now write out consolidated source
        # First, the system includes:
        for incFile in systemIncludes:
            outFile.write('#include <{}>\n'.format(incFile))
        outFile.write('\n')

        # now, write dependencies
        while len(fileGraph) != 0:
            # find an item with no dependencies
            item = findFirstFreeEntry(fileGraph)
            if item == None:
                print "Error: no free items"
                return
            
            # emit it
            node = fileGraph[item]
            outFile.write( '//\n// Contents of {}\n//\n'.format(node['filename']))
            for line in node['contents']:
                if not line.strip().startswith('#'):
                    outFile.write(line)
            
            # remove itself from all other's dependencies
            removeDependency(fileGraph, item)
    
            # remove it from graph
            del fileGraph[item]
        outFile.write('\n')
 
