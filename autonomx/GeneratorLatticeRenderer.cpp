// Copyright 2020, Xmodal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <QDebug>
#include "GeneratorLatticeRenderer.h"
#include "GeneratorLattice.h"
#include "AppModel.h"

GeneratorLatticeRenderer::GeneratorLatticeRenderer() : QQuickFramebufferObject::Renderer() {
    if(flagDebug) {
        qDebug() << "constructor (GeneratorLatticeRenderer)";
    }
    // init the shader program
    program = new QOpenGLShaderProgram();
    program->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/lattice.vert");
    program->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/lattice.frag");
    program->bindAttributeLocation("vertices", 0);
    program->link();

    QString log = program->log();
    if(flagDebug && log != "") {
        qDebug() << "constructor (GeneratorLatticeRenderer): Error in shader compilation or linking: " << log;
    }
}

GeneratorLatticeRenderer::~GeneratorLatticeRenderer() {
    // delete shader program
    delete program;
    // delete supersampling framebuffer if it exists
    if(flagSuper) {
        if(framebufferSuper != nullptr) {
            delete framebufferSuper;
        }
    }
}

void GeneratorLatticeRenderer::render() {
    if(flagDebug) {
        qDebug() << "render (GeneratorLatticeRenderer)";
    }

    // exit loop if not visible
    if(!visible) {
        if(flagDebug) {
            qDebug() << "render (GeneratorLatticeRenderer): not visible, exiting loop";
        }
        return;
    }

    // only render if lattice data is ready
    if(writeLatticeDataFirstDone) {
        // lock the lattice data mutex since we want to use that data
        generator->getLatticeMutex().lock();

        // Play nice with the RHI. Not strictly needed when the scenegraph uses
        // OpenGL directly.
        window->beginExternalCommands();

        if(flagSuper) {
            // bind supersampling framebuffer
            framebufferSuper->bind();

            // set viewport size to match supersampling framebuffer
            glViewport(0, 0, sizeSuper.width(), sizeSuper.height());
        }

        // bind shaders
        program->bind();

        // TODO: what does this do
        program->enableAttributeArray(0);

        // simple rectangle covering the whole NDC XY plane
        float values[] = {
            -1, -1,
            1, -1,
            -1, 1,
            1, 1
        };

        // This example relies on (deprecated) client-side pointers for the vertex
        // input. Therefore, we have to make sure no vertex buffer is bound.
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // TODO: what does this do
        program->setAttributeArray(0, GL_FLOAT, values, 2);

        // disable depth test
        glDisable(GL_DEPTH_TEST);

        // draw
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // we can now release the lattice data mutex
        generator->getLatticeMutex().unlock();

        // we request an update of the lattice data (as soon as possible!) if the last request finished before this frame
        if(writeLatticeDataCurrentDone) {
            writeLatticeDataCurrentDone = false;
            emit writeLatticeData(latticeData, allocatedWidth, allocatedHeight);
        }

        // TODO: what does this do
        program->disableAttributeArray(0);

        // unbind shaders
        program->release();

        if(flagSuper) {
            // release supersampling framebuffer
            framebufferSuper->release();

            // blit supersampling framebuffer onto real framebuffer, performing downsampling
            QOpenGLFramebufferObject::blitFramebuffer(framebuffer, QRect(0, 0, size.width(), size.height()), framebufferSuper, QRect(0, 0, sizeSuper.width(), sizeSuper.height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        // restore previous OpenGL state
        window->resetOpenGLState();

        // TODO: what does this do
        window->endExternalCommands();
    }

    // render again if visible
    if(visible) {
        update();
    }
}

void GeneratorLatticeRenderer::synchronize(QQuickFramebufferObject *item) {
    // sync with GeneratorLattice
    if(flagDebug) {
        qDebug() << "synchronize (GeneratorLatticeRenderer)";
    }

    // remember we are synchronizing now
    synchronized = true;

    // check if this is the first synchronization
    if(!synchronizedFirstDone) {
        synchronizedFirstDone = true;
        // update window
        window = item->window();

        // link generator
        generatorID = ((GeneratorLattice*) item)->getGeneratorID();
        generator = AppModel::getInstance().getGenerator(generatorID);

        // connect new generator
        connectionWriteLatticeData = QObject::connect(this, &GeneratorLatticeRenderer::writeLatticeData, generator, &Generator::writeLatticeData);
        connectionWriteLatticeDataCompleted = QObject::connect(generator, &Generator::writeLatticeDataCompleted, this, &GeneratorLatticeRenderer::writeLatticeDataCompleted);

        // request the lattice data
        writeLatticeDataCurrentDone = false;
        emit writeLatticeData(latticeData, allocatedWidth, allocatedHeight);
    } else {
        // check to see if the linked generator changed
        int generatorIDNew = ((GeneratorLattice*) item)->getGeneratorID();
        if(generatorIDNew != generatorID) {
            // link generator
            generatorID = generatorIDNew;
            generator = AppModel::getInstance().getGenerator(generatorID);

            // disconnect old generator
            QObject::disconnect(connectionWriteLatticeData);
            QObject::disconnect(connectionWriteLatticeDataCompleted);

            // connect new generator
            connectionWriteLatticeData = QObject::connect(this, &GeneratorLatticeRenderer::writeLatticeData, generator, &Generator::writeLatticeData);
            connectionWriteLatticeDataCompleted = QObject::connect(generator, &Generator::writeLatticeDataCompleted, this, &GeneratorLatticeRenderer::writeLatticeDataCompleted);

            // request the lattice data
            writeLatticeDataCurrentDone = false;
            emit writeLatticeData(latticeData, allocatedWidth, allocatedHeight);
        }
    }

    // get the current framebuffer, which will have changed if the window was resized
    framebuffer = this->framebufferObject();
    if(flagSuper) {
        // check to see if the framebuffer size changed
        QSize sizeNew = framebuffer->size();
        if(sizeNew != size) {
            // the size changed, we need to update the supersampling framebuffer
            size = sizeNew;
            if(flagDebug) {
                qDebug() << "render (GeneratorLatticeRenderer): synchronization caused framebuffer size change, updating supersampling framebuffer";
            }
            if(framebufferSuper != nullptr) {
                // deallocate old supersampling framebuffer if it exists
                delete framebufferSuper;
            }
            // create a new supersampling framebuffer
            sizeSuper = size * factorSuper;
            framebufferSuper = new QOpenGLFramebufferObject(sizeSuper);
        }
    }

    // update visible
    visible = item->isVisible();
    if(visible) {
        update();
    }
}

void GeneratorLatticeRenderer::writeLatticeDataCompleted() {
    writeLatticeDataCurrentDone = false;
    writeLatticeDataFirstDone = true;
}
