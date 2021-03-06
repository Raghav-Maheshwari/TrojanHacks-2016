/*
 * Copyright 2007 the original author or authors.
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

package org.gradle.api.internal;

import org.gradle.api.DefaultTask;
import org.gradle.api.Project;
import org.gradle.api.Task;
import org.gradle.api.tasks.ConventionValue;

/**
 * @author Hans Dockter
 */
public abstract class ConventionTask extends DefaultTask implements IConventionAware {
    private ConventionMapping conventionMapping;

    protected ConventionTask() {
        conventionMapping = new ConventionAwareHelper(this, getProject().getConvention());
    }

    @Deprecated
    protected ConventionTask(Project project, String name) {
        super(project, name);
        conventionMapping = new ConventionAwareHelper(this, getProject().getConvention());
    }

    public Task conventionMapping(String property, ConventionValue mapping) {
        conventionMapping.map(property, mapping);
        return this;
    }

    public ConventionMapping getConventionMapping() {
        return conventionMapping;
    }

    public void setConventionMapping(ConventionMapping mapping) {
        this.conventionMapping = mapping;
    }
}
