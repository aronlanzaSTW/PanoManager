//    PanoManager - Interactive panorama tour manager program
//    Copyright (C) 2018  Steve M Clarke
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// Scene Image
//

#include "sceneimage.h"
#include <math.h>
#include <QDir>
#include <QRgb>
#include <QFile>
#include <QProgressBar>
#include <QApplication>
#include <QStandardPaths>
#include "../errors/pmerrors.h"

SceneImage::SceneImage()
{
}


void SceneImage::clear()
{
    m_filename = "" ;
    m_facedir = "" ;
    for (int i=0; i<6; i++) {
        m_faces[i].clear() ;
    }
}

// TODO: if preview exists, and ask for load hires if avail, nothing is loaded
// loadpreview=true, buildpreview=false, buildonly=false
// loadImage increases prog by 1400
PM::Err SceneImage::loadImage(ProgressDialog *prog, QString imagefile, bool loadpreview, bool buildpreview, bool buildonly)
{
   if (!prog) return PM::InvalidPointer ;

   PM::Err err = PM::Ok ;
   QFileInfo finfo(imagefile) ;
   clear() ;
   m_filename = imagefile ;
   m_facedir = finfo.canonicalPath() + "/" + finfo.baseName();

   int progstart = prog->value() ;

   if (!facesExist(imagefile) && !buildpreview) {

       // Build Faces if requested
       err = buildFaces(prog, buildpreview) ; // 1300

   } else if (!previewExists(m_filename) && (loadpreview || buildpreview)) {

       // Build Preview if it is needed for loading, or requested for building
       err = buildFaces(prog, buildpreview) ; // 1300

   }

   prog->setValue(progstart + 1300) ;

   if (err==PM::Ok && !buildonly) {

       if (loadpreview) {

           // Load preview as requested
           err = loadFaces(true) ;

       } else {

           // Try to load faces, and on failure, load preview
           if (facesExist(imagefile)) err = loadFaces(false) ;
           if (err!=PM::Ok) err = loadFaces(true) ;

       }

   }

   prog->setValue(progstart + 1400) ;

   return err ;

}


bool SceneImage::previewExists(QString imagefile)
{
    QFileInfo f(imagefile) ;
    bool previewexists = true ;

    for (int i=0; i<6; i++) {
        QString path = f.canonicalPath() + "/" + f.baseName() + "/face00" ;
        QFile f2(path + QString::number(i) + "_preview.png") ;
        if (!f2.exists() || f2.size()==0) previewexists=false ;
    }
    return previewexists ;
}


bool SceneImage::facesExist(QString imagefile)
{
    QFileInfo f(imagefile) ;

    bool facesexist = true ;

    for (int i=0; i<6; i++) {
        QString path = f.canonicalPath() + "/" + f.baseName() + "/face00" ;
        QFile f1(path + QString::number(i) + ".png") ;
        if (!f1.exists() || f1.size()==0) facesexist=false ;
    }
    return facesexist ;
}



// Load faces, and scale to 1024x1024
PM::Err SceneImage::loadFaces(bool loadpreview)
{
    if (m_facedir.isEmpty()) return PM::InputNotDefined ;

    PM::Err err = PM::Ok ;
    m_ispreview = loadpreview ;

    for (int f=0; err==PM::Ok && f<6; f++) {
        QString path = m_facedir + "/face00" + QString::number(f) + QString(loadpreview?"_preview.png":".png") ;
        Face face ;
        if (face.load(path)) {
            m_faces[f] = face.scaled(1024, 1024) ;
        } else {
            err = PM::FaceReadError ;
        }
    }
    return err ;
}


Face &SceneImage::getFace(int n)
{
    return m_faces[n] ;
}


// Map a part of the equirectangular panorama (in) to a cube face, using the MapTranslation
// which provides the corresponding equirectangular coordinates for each face coordinate.
// buildFaces will move progress bar (prog) on by 1300.
PM::Err SceneImage::buildFaces(ProgressDialog *prog, bool buildpreview) {

    if (!prog) return PM::InvalidPointer ;

    int progstart = prog->value() ;

    QImage scaledsource ;

    // Actual equirectangular file width and height
    unsigned long int filewidth, fileheight ;
    int scaledfilewidth, scaledfileheight ;

    PM::Err err = PM::Ok ;

    {
        // Work with a smoothed and scaled version of the source for faster and more accurate colour smoothing
        QImage file ;

        if (!file.load(m_filename)) {

            err = PM::EquirectReadError ;

        } else {

            filewidth = file.width() ;
            fileheight = file.height() ;

            // Calculate the largest source scale (7 downto 1) because it
            // looks like the QImage.scaled function can't handled > 32767
            int filescale=7 ;
            do {
                scaledfilewidth = filewidth * filescale ;
                scaledfileheight = fileheight * filescale ;
                filescale-- ;
            } while ((scaledfilewidth>32767 || scaledfileheight>32767) && filescale>1) ;

            scaledsource = file.scaled(scaledfilewidth, scaledfileheight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) ;

        }
    }

    prog->setValue(progstart+100) ;

    int workingsize ;   // Size the face is generated at (3 * equirectangular image height)
    int outputsize ;    // Size the face is output at (i.e. equirectangular image height x height)

    if (buildpreview) {
        workingsize = 1024 ;
        outputsize = 512 ;
    } else {
        // Use prime number for working size multiplier as better smooting achieved
        workingsize = fileheight * 3 ;
        outputsize = fileheight ;
    }

    QDir dir ;

    if (err==PM::Ok && !dir.exists(m_facedir) && !dir.mkdir(m_facedir)) {
        err = PM::OutputWriteError ;
    }

    for (int f=0; err==PM::Ok && f<6; f++) {
        if (err==PM::Ok) {
            Face face ;
            prog->setText2(QString("Building face ") + QString::number(f)) ;
            err = face.build(prog, scaledsource, f, workingsize) ; // 200
            if (err==PM::Ok) {
                face = face.scaled(fileheight, fileheight) ;
                face.save(m_facedir + "/face00" + QString::number(f) + QString(buildpreview?"_preview.png":".png")) ;
                if (prog->isCancelled()) err=PM::OperationCancelled ;
            }
        }
    }

    return err ;
}
