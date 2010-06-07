//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef BACKGROUNDSPROPERTYBROWSER_HH
#define BACKGROUNDSPROPERTYBROWSER_HH

namespace redshift {
        namespace scenefile {
                class Background;
        }
}
class QWidget;
class QVariant;
class QtTreePropertyBrowser;
class QtVariantPropertyManager;
class QtVariantEditorFactory;
class QtGroupPropertyManager;
class QtProperty;


#include <QObject>
#include <QList>

class BackgroundsPropertyBrowser : public QObject
{
        Q_OBJECT
public:
        BackgroundsPropertyBrowser(QWidget *,
                                   QtTreePropertyBrowser *);

        // this method won't have a long stay once multiple backgrounds
        // (consider e.g. preetham + starfield + moon) are supported
        void setBackground (redshift::scenefile::Background const &);

        QList<QtProperty*> subProperties();
signals:
        void sceneChanged();
        void updateUi();
private:
        void initialize();

        QWidget *ownerWidget;
        QtTreePropertyBrowser *root;

        QtProperty *backgroundsProperty,
                   *pssSunSkyProperty;

        QtGroupPropertyManager *groupManager;
        QtVariantPropertyManager *variantManager;
        QtVariantEditorFactory *variantFactory;
private slots:
        void variantManager_valueChanged(
                        QtProperty*, QVariant const &);
};

#endif // BACKGROUNDSPROPERTYBROWSER_HH