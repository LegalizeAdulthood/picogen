#include "qmessagebox.h"

#include "mainwindow.hh"
#include "ui_mainwindow.h"

#include "qtwindowlistmenu.h"
#include "qtvariantproperty.h"
#include "qtpropertymanager.h"
#include "qttreepropertybrowser.h"
#include <QtStringPropertyManager>

#include "simpleinputbox.hh"
#include "renderwindow.hh"
#include "quatschsourceeditor.hh"

#include "redshift/include/jobfile.hh"


namespace {

        // Some helper functions for the lack of a method in QtProperty
        // that lets us select sub-items directly by name.

        template <typename T>
        T readValue (QString name, QList<QtProperty*> list) {
                QtProperty* retty = 0;
                foreach (QtProperty* looky, list) {
                        if (name == looky->propertyName()) {
                                retty = looky;
                                break;
                        }
                }
                if (0 != retty) {
                        const QtVariantProperty *v = (QtVariantProperty*)retty;
                        const QVariant vv = v->value();
                        const T ret = vv.value<T>();
                        return ret;
                }
                return T();
        }


        QString readValueText (QString name, QList<QtProperty*> list) {
                foreach (QtProperty* looky, list) {
                        if (name == looky->propertyName()) {
                                return looky->valueText();
                        }
                }
                return "";
        }


        QList<QtProperty*> readSubProperties (QString name, QList<QtProperty*> list) {
                foreach (QtProperty* looky, list) {
                        if (name == looky->propertyName()) {
                                return looky->subProperties();
                        }
                }
                return QList<QtProperty*>();
        }


        QtProperty *findParent(QtProperty *root, QtProperty* child) {
                if (root == child)
                        return 0;
                foreach (QtProperty *prop, root->subProperties()) {
                        if(prop == child)
                                return root;
                        if (QtProperty *n = findParent(prop, child))
                                return n;
                }
                return 0;
        }

        QtProperty *findParent(QList<QtProperty*> props, QtProperty* child) {
                foreach (QtProperty *prop, props) {
                        if (QtProperty *curr = findParent (prop, child))
                                return curr;
                }
                return 0;
        }
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentBrowserItem(0),
    nonRecurseLock(false)
{
        ui->setupUi(this);

        // code editor
        ui->codeEditor->setEnabled(false);
        ui->codeEditor->setVisible(false);

        // WindowList
        QtWindowListMenu *winMenu = new QtWindowListMenu(menuBar());
        winMenu->attachToMdiArea(ui->mdiArea);
        winMenu->setTitle("Windows");
        menuBar()->addMenu(winMenu);

        // Properties
        variantManager = new QtVariantPropertyManager(this);
        rsTitleManager = new QtStringPropertyManager (this);
        connect(rsTitleManager, SIGNAL(valueChanged (QtProperty *, const QString &)),
                this, SLOT(rsTitleManager_valueChanged(QtProperty*,const QString &)));

        groupManager = new QtGroupPropertyManager(this);
        enumManager = new QtEnumPropertyManager(this);


        transformEnumManager = new QtEnumPropertyManager(this);
        connect(transformEnumManager, SIGNAL(valueChanged (QtProperty *, int)),
                this, SLOT(transformEnumManager_valueChanged(QtProperty*,int)));

        objectTypeEnumManager = new QtEnumPropertyManager(this);
        connect(objectTypeEnumManager, SIGNAL(valueChanged (QtProperty *, int)),
                this, SLOT(objectTypeEnumManager_valueChanged(QtProperty*,int)));

        codeEditManager = new QtVariantPropertyManager(this);
        connect(codeEditManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
                this, SLOT(code_valueChanged(QtProperty*, QVariant)));

        comboBoxFactory = new QtEnumEditorFactory(this);
        lineEditFactory = new QtLineEditFactory(this);

        variantFactory = new QtVariantEditorFactory(this);
        ui->settings->setFactoryForManager(variantManager, variantFactory);
        ui->settings->setFactoryForManager(enumManager, comboBoxFactory);
        ui->settings->setFactoryForManager(transformEnumManager, comboBoxFactory);
        ui->settings->setFactoryForManager(objectTypeEnumManager, comboBoxFactory);
        ui->settings->setFactoryForManager(rsTitleManager, lineEditFactory);

        // Film Settings.
        {
                QtProperty *topItem = 0;
                QtVariantProperty *item = 0;

                topItem = groupManager->addProperty(QLatin1String("film-settings"));

                // convert-to-srgb
                item = variantManager->addProperty(
                                QVariant::Bool,
                                QLatin1String("convert-to-srgb"));
                item->setValue(true);
                topItem->addSubProperty(item);

                // color-scale
                item = variantManager->addProperty(
                                QVariant::Double,
                                QLatin1String("color-scale"));
                item->setValue(0.017);
                item->setAttribute(QLatin1String("singleStep"), 0.01);
                item->setAttribute(QLatin1String("decimals"), 6);
                item->setAttribute(QLatin1String("minimum"), 0);
                item->setAttribute(QLatin1String("maximum"), redshift::constants::infinity);
                topItem->addSubProperty(item);


                ui->settings->addProperty(topItem);
        }

        // Render Settings.
        {
                QtProperty *topItem = groupManager->addProperty("render-settings");
                renderSettingsProperty = topItem;
                ui->settings->addProperty(topItem);

                addRenderSettings("preview");
                addRenderSettings("production");
        }

        // Camera Settings.
        {
                camerasProperty = groupManager->addProperty("cameras");
                ui->settings->addProperty(camerasProperty);
                addCamera("hello-world");
        }

        // Objects.
        {
                objectsProperty = groupManager->addProperty("objects");
                ui->settings->addProperty(objectsProperty);

                addObject();
        }

        ui->settings->setRootIsDecorated(true);
        //ui->settings->setIndentation(32);
        ui->settings->setHeaderVisible(false);



        //menuBar()->repaint();
}



MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::addRenderSettings (std::string const &name) {

        QtProperty *topItem = groupManager->addProperty(name.c_str());

        QtProperty *title = rsTitleManager->addProperty("title");
        rsTitleManager->setRegExp(title, QRegExp("([a-z0-9]|-|_)+", Qt::CaseInsensitive, QRegExp::RegExp));
        rsTitleManager->setValue(title, name.c_str());
        topItem->addSubProperty(title);


        QtVariantProperty *it = variantManager->addProperty(QVariant::Int, "width");
        it->setAttribute(QLatin1String("minimum"), 1);
        it->setAttribute(QLatin1String("maximum"), 0xFFFFFF);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(320);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "height");
        it->setAttribute(QLatin1String("minimum"), 1);
        it->setAttribute(QLatin1String("maximum"), 0xFFFFFF);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(240);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "samples-per-pixel");
        it->setAttribute(QLatin1String("minimum"), 1);
        it->setAttribute(QLatin1String("maximum"), 0xFFFFFF);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(1);
        topItem->addSubProperty(it);


        QtProperty *volumeIntegrator = groupManager->addProperty("volume-integrator");
        topItem->addSubProperty(volumeIntegrator);

        QtProperty *integratorType = enumManager->addProperty("type");
        QStringList enumNames;
        enumNames << "none" << "emission" << "single";
        enumManager->setEnumNames(integratorType, enumNames);
        volumeIntegrator->addSubProperty(integratorType);
        it = variantManager->addProperty(QVariant::Double, "step-size");

        it->setAttribute(QLatin1String("minimum"), 1.);
        it->setAttribute(QLatin1String("maximum"), 32768.);
        it->setAttribute(QLatin1String("singleStep"), 1.);
        it->setAttribute(QLatin1String("decimals"), 1);
        it->setValue(500);
        volumeIntegrator->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Double, "cutoff-distance");
        it->setAttribute(QLatin1String("minimum"), 1.);
        it->setAttribute(QLatin1String("maximum"), 32768.);
        it->setAttribute(QLatin1String("singleStep"), 1.);
        it->setAttribute(QLatin1String("decimals"), 1);
        volumeIntegrator->addSubProperty(it);

        renderSettingsProperty->addSubProperty(topItem);
}



void MainWindow::addCamera(const std::string &name) {
        QtProperty *camera = groupManager->addProperty(name.c_str());

        QtProperty *transformRoot = groupManager->addProperty("transform");

        camera->addSubProperty(transformRoot);
        camerasProperty->addSubProperty(camera);

        addTransform (transformRoot, redshift::scenefile::Camera::Transform::move);
        addTransform (transformRoot, redshift::scenefile::Camera::Transform::yaw);
        addTransform (transformRoot, redshift::scenefile::Camera::Transform::pitch);
        addTransform (transformRoot, redshift::scenefile::Camera::Transform::roll);
}



void MainWindow::addTransform (QtProperty *transformRoot,
                               redshift::scenefile::Camera::Transform::Type type) {
        typedef redshift::scenefile::Camera::Transform Xf;

        QtProperty *transform = groupManager->addProperty("---");
        transformRoot->addSubProperty(transform);


        QtProperty *transformType = transformEnumManager->addProperty("type");
        transform->addSubProperty(transformType);

        QStringList enumNames;
        int i = 0, def = 0;
        enumNames << "move";
        if (type == Xf::move) def = i; i++;
        enumNames << "move-left";
        if (type == Xf::move_left) def = i; i++;
        enumNames << "move-right";
        if (type == Xf::move_right) def = i; i++;
        enumNames << "move-up";
        if (type == Xf::move_up) def = i; i++;
        enumNames << "move-backward";
        if (type == Xf::move_backward) def = i; i++;
        enumNames << "move-forward";
        if (type == Xf::move_forward) def = i; i++;
        enumNames << "yaw";
        if (type == Xf::yaw) def = i; i++;
        enumNames << "pitch";
        if (type == Xf::pitch) def = i; i++;
        enumNames << "roll";
        if (type == Xf::roll) def = i; i++;
        transformEnumManager->setEnumNames(transformType, enumNames);
        transformEnumManager->setValue(transformType, def);
}



void MainWindow::addObject () {
        QtProperty *object = groupManager->addProperty("---");
        objectsProperty->addSubProperty(object);

        QtProperty *objectType = objectTypeEnumManager->addProperty("type");
        object->addSubProperty(objectType);


        QStringList enumNames;
        enumNames << "horizon-plane"
                  << "water-plane"
                  << "lazy-quadtree"
                  ;
        objectTypeEnumManager->setEnumNames(objectType, enumNames);
}



void MainWindow::transformEnumManager_valueChanged(
        QtProperty* prop,
        int index
) {
        if (QtProperty *t = findParent(ui->settings->properties(), prop)) {
                const QStringList enumNames =
                                transformEnumManager->enumNames(prop);
                const QString type = enumNames[index];

                t->setPropertyName(type);

                // Remove all properties not titled "type".
                foreach (QtProperty *nt, t->subProperties()){
                        if (nt->propertyName() != "type") {
                                t->removeSubProperty(nt);
                        }
                }

                // When tweaking here, also look at XCVBN
                if (type == "move") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"right"));
                        QtVariantProperty *tmp = variantManager->addProperty(QVariant::Double,"up");
                        tmp->setValue(1.);
                        t->addSubProperty(tmp);
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"forward"));
                } else if (type == "move-left") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"distance"));
                } else if (type == "move-right") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"distance"));
                } else if (type == "move-up") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"distance"));
                } else if (type == "move-down") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"distance"));
                } else if (type == "move-forward") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"distance"));
                } else if (type == "move-backward") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"distance"));
                } else if (type == "yaw") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"angle"));
                } else if (type == "pitch") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"angle"));
                } else if (type == "roll") {
                        t->addSubProperty(variantManager->addProperty(QVariant::Double,"angle"));
                } else {
                        throw std::runtime_error(
                           (QString()
                           + "MainWindow::transformEnumManager_valueChanged(): type '"
                           + type
                           + "' not supported").toStdString().c_str());
                }

        }
}



void MainWindow::objectTypeEnumManager_valueChanged (
        QtProperty* prop,
        int index
) {
        // parent
        //   |
        //   +-----type
        //   |
        //   +-----param*

        // We have 'type', now find 'parent'
        if (QtProperty *t = findParent(ui->settings->properties(), prop)) {
                const QStringList enumNames =
                                objectTypeEnumManager->enumNames(prop);
                const QString type = enumNames[index];

                t->setPropertyName(type);

                // Remove all properties not titled "type".
                foreach (QtProperty *nt, t->subProperties()){
                        if (nt->propertyName() != "type") {
                                t->removeSubProperty(nt);
                        }
                }

                if (type == "horizon-plane") {
                        QtVariantProperty* it = variantManager->addProperty(QVariant::Double,"height");
                        t->addSubProperty(it);
                        it->setAttribute(QLatin1String("decimals"), 6);
                } else if (type == "water-plane") {
                        QtVariantProperty* height = variantManager->addProperty(QVariant::Double,"height");
                        t->addSubProperty(height);
                        height->setAttribute(QLatin1String("decimals"), 6);
                        QtProperty* code = codeEditManager->addProperty(QVariant::String, "code");
                        t->addSubProperty(code);
                } else if (type == "lazy-quadtree") {
                        t->addSubProperty(variantManager->addProperty(QVariant::UserType,"color"));

                        QtProperty* code = codeEditManager->addProperty(QVariant::String, "code");
                        t->addSubProperty(code);

                        QtVariantProperty* maxrec = variantManager->addProperty(QVariant::Int,"max-recursion");
                        maxrec->setToolTip("Be careful. Slowly approach values beyond 10. For preview renderings, values less than 10 seem to work well.");
                        maxrec->setAttribute(QLatin1String("minimum"), 1);
                        maxrec->setAttribute(QLatin1String("maximum"), 20);
                        maxrec->setValue(7);
                        t->addSubProperty(maxrec);

                        QtVariantProperty* lodfac = variantManager->addProperty(QVariant::Double,"lod-factor");
                        lodfac->setToolTip("Very sensible factor. The smaller, the less detail-loss upon increasing distance.");
                        lodfac->setAttribute(QLatin1String("minimum"), 0.000001);
                        lodfac->setAttribute(QLatin1String("maximum"), 1);
                        lodfac->setAttribute(QLatin1String("singleStep"), 0.00001);
                        lodfac->setAttribute(QLatin1String("decimals"), 6);
                        lodfac->setValue(0.000125);
                        t->addSubProperty(lodfac);

                        QtVariantProperty* size = variantManager->addProperty(QVariant::Double,"size");
                        lodfac->setToolTip("Picogen's quadtree scales quite well. Feel free to try out very large values of 100k and bigger.");
                        size->setAttribute(QLatin1String("minimum"), 1);
                        size->setAttribute(QLatin1String("maximum"), redshift::constants::infinity);
                        size->setAttribute(QLatin1String("singleStep"), 1000);
                        maxrec->setAttribute(QLatin1String("decimals"), 1);
                        size->setValue(50000);
                        t->addSubProperty(size);

                        //t->addSubProperty(variantManager->addProperty(QVariant::V,"color"));
                }
        }
}



void MainWindow::rsTitleManager_valueChanged (
        QtProperty * property,
        const QString & value
) {
        if (QtProperty *par = findParent (ui->settings->properties(), property)) {
                par->setPropertyName(value);
        }
}



void MainWindow::changeEvent(QEvent *e) {
        QMainWindow::changeEvent(e);
        switch (e->type()) {
        case QEvent::LanguageChange:
                ui->retranslateUi(this);
                break;
        default:
                break;
        }
}



void MainWindow::on_settings_currentItemChanged(QtBrowserItem * current) {

        this->currentBrowserItem = current;
        QString name = (current==0) ? "" : current->property()->propertyName();

        // TODO: below are a bit fragile, but for now work very well.
        ui->newTransformButton->setEnabled(name == "transform");

        const bool isCode = name == "code";
        if (isCode) {
                ui->codeEditor->setEnabled(true);
                ui->codeEditor->setVisible(true);
                ui->codeEditor->setCode(codeEditManager->value(current->property()).toString());
        } else {
                ui->codeEditor->setEnabled(false);
                ui->codeEditor->setVisible(false);
        }

        const QtProperty *parentProp = (current==0)
                                     ? 0
                                     : findParent(ui->settings->properties(),
                                                  current->property());

        ui->deleteObjectButton->setEnabled(
                        (parentProp != 0)
                        && (parentProp->propertyName() == "objects"));
}



redshift::shared_ptr<redshift::scenefile::Scene>
        MainWindow::createScene () const
{
        typedef QList<QtProperty*> Props;
        typedef QtProperty* Prop;
        typedef QtVariantProperty* VProp;


        Props topProps = ui->settings->properties();

        QString ret;
        using namespace redshift;
        shared_ptr<scenefile::Scene> scene = shared_ptr<scenefile::Scene>(
                        new scenefile::Scene());

        // FilmSettings.
        const Props filmSettings = readSubProperties("film-settings", topProps);
        scenefile::FilmSettings fs;
        fs.colorscale = readValue<double>("color-scale", filmSettings);
        fs.convertToSrgb = readValue<bool>("convert-to-srgb", filmSettings);
        scene->setFilmSettings(fs);

        // RenderSettings.
        const Props renderSettings = readSubProperties("render-settings", topProps);
        foreach (Prop mooh, renderSettings) {
                Props subs = mooh->subProperties();

                scenefile::RenderSettings rs;
                rs.width = readValue<unsigned int>("width", subs);
                rs.height = readValue<unsigned int>("height", subs);
                rs.samplesPerPixel = readValue<unsigned int>("samples-per-pixel", subs);

                Props vp = readSubProperties("volume-integrator", subs);
                rs.volumeIntegrator.stepSize = readValue<double>("step-size", vp);
                rs.volumeIntegrator.cutoffDistance = readValue<double>("cutoff-distance", vp);
                const QString integT = readValueText("type", vp);
                if ("none" == integT)
                        rs.volumeIntegrator.type = scenefile::VolumeIntegrator::none;
                else if ("emission" == integT)
                        rs.volumeIntegrator.type = scenefile::VolumeIntegrator::emission;
                else if ("single" == integT)
                        rs.volumeIntegrator.type = scenefile::VolumeIntegrator::single;

                scene->addRenderSettings(rs);
        }

        // Camera.
        const Props cameras = readSubProperties("cameras", topProps);
        foreach (Prop cam, cameras) {
                scenefile::Camera camera;
                camera.title = cam->propertyName().toStdString();

                const Props xforms = readSubProperties("transform", cam->subProperties());
                foreach (Prop xform, xforms) {
                        typedef scenefile::Camera::Transform Xf;
                        Xf transform;

                        // When tweaking here, also look at XCVBN
                        const Props xfsubs = xform->subProperties();
                        const QString type = readValueText("type", xfsubs);


                        if (type == "move") {
                                transform.type = Xf::move;
                                transform.x = readValue<double>("right", xfsubs);
                                transform.y = readValue<double>("up", xfsubs);
                                transform.z = readValue<double>("forward", xfsubs);
                        } else if (type == "move-left") {
                                transform.type = Xf::move_left;
                                transform.x = readValue<double>("distance", xfsubs);
                        } else if (type == "move-right") {
                                transform.type = Xf::move_right;
                                transform.x = readValue<double>("distance", xfsubs);
                        } else if (type == "move-up") {
                                transform.type = Xf::move_up;
                                transform.y = readValue<double>("distance", xfsubs);
                        } else if (type == "move-down") {
                                transform.type = Xf::move_down;
                                transform.y = readValue<double>("distance", xfsubs);
                        } else if (type == "move-forward") {
                                transform.type = Xf::move_forward;
                                transform.z = readValue<double>("distance", xfsubs);
                        } else if (type == "move-backward") {
                                transform.type = Xf::move_backward;
                                transform.z = readValue<double>("distance", xfsubs);
                        } else if (type == "yaw") {
                                transform.type = Xf::yaw;
                                transform.angle = readValue<double>("angle", xfsubs);
                        } else if (type == "pitch") {
                                transform.type = Xf::pitch;
                                transform.angle = readValue<double>("angle", xfsubs);
                        } else if (type == "roll") {
                                transform.type = Xf::roll;
                                transform.angle = readValue<double>("angle", xfsubs);
                        } else {
                                throw std::runtime_error(
                                   (QString()
                                   + "MainWindow::createScene () const: transform-type '"
                                   + type
                                   + "' not supported").toStdString().c_str());
                        }
                        camera.transforms.push_back(transform);
                }

                scene->addCamera(camera);
        }


        // Objects.
        const Props objects = readSubProperties("objects", topProps);
        foreach (Prop object, objects) {
                using scenefile::Object;

                const Props subs = object->subProperties();
                const QString type = readValueText("type", subs);

                scenefile::Object object;

                if (type == "water-plane") {
                        object.type = Object::water_plane;
                        const std::string tmp = readValue<QString>("code", subs).toStdString();
                        if(!tmp.empty())
                                object.waterPlaneParams.code = tmp;
                        //object.waterPlaneParams.color
                        object.waterPlaneParams.height = readValue<double>("height", subs);
                } else if (type == "horizon-plane") {
                        object.type = Object::horizon_plane;
                        //object.horizonPlaneParams.color
                        object.horizonPlaneParams.height = readValue<double>("height", subs);
                } else if (type == "lazy-quadtree") {
                        object.type = Object::lazy_quadtree;
                        const std::string tmp = readValue<QString>("code", subs).toStdString();
                        if(!tmp.empty())
                                object.lazyQuadtreeParams.code = tmp;
                        //object.lazyQuadtreeParams.color;
                        object.lazyQuadtreeParams.lodFactor = readValue<double>("lod-factor", subs);
                        object.lazyQuadtreeParams.maxRecursion = readValue<unsigned int>("max-recursion", subs);
                        object.lazyQuadtreeParams.size = readValue<double>("size", subs);
                } else {
                        throw std::runtime_error(
                           (QString()
                           + "MainWindow::createScene () const: object-type '"
                           + type
                           + "' not supported").toStdString().c_str());
                }

                scene->addObject(object);

        }


        return scene;
}



void MainWindow::on_actionShow_redshift_job_code_triggered() {
        using namespace actuarius;
        using namespace redshift;

        const shared_ptr<scenefile::Scene> scene = createScene ();
        std::stringstream ss;
        OArchive (ss) & pack("scene", *scene);
        QMessageBox::information(this, QString("Teh codes"), ss.str().c_str());
}



void MainWindow::on_actionRender_triggered() {
        RenderWindow *rw = new RenderWindow (createScene(), this);
        ui->mdiArea->addSubWindow(rw);
        rw->show();
}



void MainWindow::on_newRsButton_pressed() {
        addRenderSettings ("new_setting");
}



void MainWindow::on_newTransformButton_pressed() {
        // We assume that newTransform can only clicked when the current-item
        // is a transform.
        addTransform (currentBrowserItem->property(),
                      redshift::scenefile::Camera::Transform::move);
}



void MainWindow::on_newObjectButton_pressed() {
        addObject ();
}



void MainWindow::on_codeEditor_codeChanged() {
        // We assume that this can only be triggered if the current item
        // is some code.
        if (nonRecurseLock)
                return;
        nonRecurseLock = true;
        codeEditManager->setValue(currentBrowserItem->property(),
                                  ui->codeEditor->code());
        nonRecurseLock = false;
}



void MainWindow::code_valueChanged(QtProperty*, QVariant code) {
        // This is under the assumption that the code window is open for the
        // property that just emitted this signal.
        if (nonRecurseLock)
                return;
        nonRecurseLock = true;
        ui->codeEditor->setCode(code.toString());
        nonRecurseLock = false;
}



void MainWindow::on_deleteObjectButton_pressed() {
        // assumed to signal everything needed for clean up
        objectsProperty->removeSubProperty(currentBrowserItem->property());
}
