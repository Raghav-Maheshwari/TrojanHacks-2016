/*
 * Copyright 2009 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.gradle.api.changedetection.state;

import org.apache.commons.io.IOUtils;

import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.FileWriter;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

/**
 * @author Tom Eyckmans
 */
class DirectoryListFileCreator {

    private final StateFileUtil stateFileUtil;
    private final List<BufferedWriter> dirListLevelWriters;
    private int lowestLevel = 0;

    DirectoryListFileCreator(StateFileUtil stateFileUtil) {
        this.stateFileUtil = stateFileUtil;
        this.dirListLevelWriters = new ArrayList<BufferedWriter>();
    }

    BufferedWriter getDirListLevelWriter(int levelIndex) throws IOException {
        BufferedWriter dirListLevelWriter = null;

        if ( dirListLevelWriters.size() > levelIndex ) {
            dirListLevelWriter = dirListLevelWriters.get(levelIndex);
        }
        else {
            dirListLevelWriter = new BufferedWriter(new FileWriter(stateFileUtil.getDirsListFile(levelIndex)));
            dirListLevelWriters.add(dirListLevelWriter);
            lowestLevel = levelIndex;
        }

        return dirListLevelWriter;
    }

    public int createDirectoryListFiles(final File directory) throws IOException {
        try {
            fillDirectoryListFiles(directory, 0);
        }
        finally {
            for ( final BufferedWriter dirListLevelWriter : dirListLevelWriters ) {
                IOUtils.closeQuietly(dirListLevelWriter);
            }
        }

        return lowestLevel;
    }

    public void fillDirectoryListFiles(final File directory, final int level) throws IOException {
        final File[] subFiles = directory.listFiles();

        final BufferedWriter dirListLevelWriter = getDirListLevelWriter(level);

        dirListLevelWriter.write(directory.getAbsolutePath());
        dirListLevelWriter.newLine();

        if ( subFiles != null && subFiles.length > 0 ) {
            final List<File> subDirectories = new ArrayList<File>();
            for ( final File subFile : subFiles ) {
                if ( subFile.isDirectory() ) {
                    subDirectories.add(subFile);
                }
                // ignore files
            }

            Collections.sort(subDirectories, new Comparator<File>() {
                public int compare(final File firstDirectory, final File secondDirectory) {
                    final String relativeFirstDirectoryPath = stateFileUtil.getRelativePathToDirectoryToProcess(firstDirectory);
                    final String relativeSecondDirectoryPath = stateFileUtil.getRelativePathToDirectoryToProcess(secondDirectory);

                    return relativeFirstDirectoryPath.compareTo(relativeSecondDirectoryPath);
                }
            });

            for ( final File subDirectory : subDirectories ) {
                fillDirectoryListFiles(subDirectory, level + 1);
            }
        }
    }
}
