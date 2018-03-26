// Copyright 2016 ESRI
//
// All rights reserved under the copyright laws of the United States
// and applicable international laws, treaties, and conventions.
//
// You may freely redistribute and use this sample code, with or
// without modification, provided you include the original copyright
// notice and use restrictions.
//
// See the Sample code usage restrictions document for further information.
//

#include "Map.h"
#include "MapQuickView.h"
#include "Basemap.h"
#include "PopupManager.h"
#include "MobileMapPackage.h"
#include "FeatureTableListModel.h"
#include "AttributeListModel.h"
#include "FeatureTable.h"
#include "FeatureLayer.h"
#include "OfflineMapTask.h"
#include "AuthenticationManager.h"
#include "OfflineMapSyncTask.h"
#include "OfflineMapSyncParameters.h"
#include "QTimer.h"

#include "Cleanup.h"
#include "attributetable.h"

#include <QQmlEngine>
#include <QTableView>
#include <QDir>
#include <windows.h>

using namespace Esri::ArcGISRuntime;

//PURPOSE: Class constructor
//PARAMETERS: N/A
//RETURN: N/A
Cleanup::Cleanup(QQuickItem* parent /* = nullptr */):
    QQuickItem(parent),
    m_map(nullptr),
    m_offlineMap(nullptr),
    m_mapView(nullptr),
    m_mapDrawing(false),
    m_legendInfoListModel(nullptr),
    m_puManager(nullptr),
    m_offlineMapTask(nullptr),
    m_offlineMapSyncTask(nullptr),
    iterOffset(0),
    currentTable(0),
    m_syncProgress(0.0),
    m_syncText(),
    syncInterval(30000)
{
    init();
}

//PURPOSE: Class destructor
//PARAMETERS: N/A
//RETURN: N/A
Cleanup::~Cleanup()
{
}

//PURPOSE: Runs on object creation
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::init(){
    qmlRegisterType<MapQuickView>("Esri.Cleanup", 1, 0, "MapView");
    qmlRegisterType<Cleanup>("Esri.Cleanup", 1, 0, "Cleanup");
    qmlRegisterUncreatableType<AuthenticationManager>("Esri.Cleanup", 1, 0, "AuthenticationManager*", "Uncreatable");
    qmlRegisterUncreatableType<PopupManager>("Esri.Cleanup", 1, 0, "PopupManager", "Uncreatable");
    qmlRegisterUncreatableType<LegendInfoListModel>("Esri.Cleanup", 1, 0, "LegendInfoListModel", "Uncreatable");
    qmlRegisterUncreatableType<AttributeListModel>("Esri.Cleanup", 1, 0, "AttributeListModel", "Uncreatable");
}

//PURPOSE: Runs on startup
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::componentComplete()
{
    QQuickItem::componentComplete();

    //Get data path of MMPK and the URL of feature service
    QString dataPath = QDir::currentPath();
    mmpkPath = dataPath + "/Cleanup_MMPK";
    m_map = new Map(QUrl("http://gocolumbiamo.maps.arcgis.com/home/item.html?id=45414a367c7742f395f2b2cb1421337a"));

    //find QML MapView component
    m_mapView = findChild<MapQuickView*>("mapView");
    m_mapView->setWrapAroundMode(WrapAroundMode::Disabled);

    //bring in MMPK
    if(QDir(mmpkPath.toUtf8()).exists()){
        m_mobileMapPackage = new MobileMapPackage(mmpkPath, this);
        connect(m_mobileMapPackage, &MobileMapPackage::doneLoading, this,
                [this](Error loadError){
            if(!loadError.isEmpty()){qDebug() << loadError.message();}
            m_offlineMap = m_mobileMapPackage->maps().at(0);
            m_offlineMapSyncTask = new OfflineMapSyncTask(m_offlineMap, this);
            offlineMapSyncTask_Connections();//Setup offline map connections
            m_offlineMapSyncTask->load();//Load offline map
            m_mapView->setMap(m_offlineMap);//Set mapView to offline map
            makeConnections();//Setup signal connections
        });
        m_mobileMapPackage->load();
    }else{//WILL ONLY HAPPEN IF YOU SOMEHOW DELETE YOUR MMPK OR NEED TO DOWNLOAD IT FOR THE FIRST TIME
        connect(m_map, &Map::doneLoading, this,
                [this](Error loadError){
            if(!loadError.isEmpty()){qDebug() << loadError.message();}
            m_offlineMapTask = new OfflineMapTask(m_map, this);
            offlineMapTask_Connections();//Setup offline map connections
            m_offlineMapTask->load();//Load offline map
        });
        m_map->load();//Load online map
    }

    //Listen for map changes
    connect(m_mapView, &MapQuickView::drawStatusChanged, this, [this](DrawStatus drawStatus){
        if(drawStatus == DrawStatus::InProgress){ m_mapDrawing = true; }
        else{ m_mapDrawing = false; }
        emit mapDrawStatusChanged();
    });
}

//PURPOSE: Run the query from the search bar
//PARAMETERS: The string that was entered into the search bar
//RETURN: N/A
void Cleanup::runQuery(const QString &qInput, const QString &trigger){
    emit clearResults();
    QueryParameters queryParams;
    QString convInput = qInput;
    convInput = convInput.replace(QRegExp("\'"), "\'\'");
    QStringList convList = convInput.split(QRegExp(" - "));
    if(trigger == "search"){
        queryParams.setWhereClause("Grid_Id LIKE '%" + convInput.toUpper().toUtf8() + "%' OR " +
                                   "Group_Name LIKE '%" + convInput.toUtf8() + "%'");//Creating query params
    }else if(trigger == "table"){
        queryParams.setWhereClause("GlobalID LIKE '" + convInput.toUtf8() + "'");
    }else if(trigger == "suggest"){
        queryParams.setWhereClause("Group_Name LIKE '%" + convList[0].toUtf8() + "%'");//Creating query params
    }
    FeatureLayer* fl(nullptr);
    failureCnt = iterOffset;
    for(int i = iterOffset; i < m_offlineMap->operationalLayers()->size(); i++){
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(i));
        if(trigger == "suggest" && convList[1] == fl->name()){
            fl->selectFeatures(queryParams, SelectionMode::New);//Select from layers based on query params
        }else if(trigger != "suggest"){
            fl->selectFeatures(queryParams, SelectionMode::New);//Select from layers based on query params
        }
    }
}

//PURPOSE: Create signals for offline map sync task
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::offlineMapSyncTask_Connections(){
    connect(m_offlineMapSyncTask, &OfflineMapSyncTask::doneLoading, this,
            [this](Error loadError){
        if(!loadError.isEmpty()){qDebug() << loadError.message();}
        qDebug() << "Done loading offline map sync task";
        syncDown();//Start syncing with feature service by downloading differences
    });
}

//PURPOSE: Create signals for maps
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::makeConnections(){
    m_offlineMap->setAutoFetchLegendInfos(true);

    listen_Identify();//Setup listeners for feature identification
    listen_Query();//Setup listeners for feature querying

    //listen_Table();//Setup listeners for attribute table
    //buildTable();//Build the attribute table for the first time

    connect(m_offlineMap->legendInfos(), &LegendInfoListModel::fetchLegendInfosCompleted, this, [this](){
        m_legendInfoListModel = m_offlineMap->legendInfos();
        emit legendInfoListModelChanged();
    });

    connect(m_mapView, &MapQuickView::mouseClicked, this, [this](QMouseEvent& mouseEvent){//When you click on a feature, identify it
        for(int i = iterOffset; i < m_offlineMap->operationalLayers()->size(); i++){
            m_mapView->identifyLayer(m_offlineMap->operationalLayers()->at(i), mouseEvent.x(), mouseEvent.y(), 10, true);
        }
    });

    QTimer *timer = new QTimer(this);//Setup timer for periodic syncing
    connect(timer, &QTimer::timeout, this, [this](){
        syncUpAndDown();//Start syncing with feature service
    });
    timer->start(syncInterval);//Use sync interval defined above (By default 30s) to refresh map with feature service
}

//PURPOSE: Listener for feature querying
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::listen_Query(){
    FeatureLayer* fl(nullptr);
    for(int i = iterOffset; i < m_offlineMap->operationalLayers()->size(); i++){
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(i));
        connect(fl, &FeatureLayer::selectFeaturesCompleted, this, [this, fl](QUuid, Esri::ArcGISRuntime::FeatureQueryResult* fqResult){
            for(int j = 0; j < fqResult->iterator().features().size(); j++){
                QString gName = fqResult->iterator().features().at(j)->attributes()->attributeValue("Group_Name").toString();
                if(gName != ""){
                    gName = gName + " - " + fl->name();
                    emit fqResultAdded(gName);
                }
            }
            if(!fqResult->iterator().features().empty()){
                //qDebug("Triggered Query Listener!!!");
                connect(m_mapView, &MapQuickView::setViewpointCompleted, this, [this, fl, fqResult](bool succeeded){
                    if(succeeded){
                        Multipart* mp = new Multipart(fqResult->iterator().features().at(0)->geometry());
                        QPointF screenPt = m_mapView->locationToScreen(mp->parts().part(0).point(0));
                        m_mapView->identifyLayer(fl, screenPt.x(), screenPt.y(), 0, true);
                    }
                });
                m_mapView->setViewpointGeometry(fqResult->iterator().features().at(0)->geometry(), 100);
            }else{failureCnt++;}
            if(failureCnt >= m_offlineMap->operationalLayers()->size()){
                m_queryFailure = true;
            }else{
                m_queryFailure = false;
            }emit queryFailureChanged();
        });
    }
}

//PURPOSE: Listener for attribute table
//PARAMETERS: N/A
//RETURN: N/A
/*void Cleanup::listen_Table(){
    FeatureLayer* fl(nullptr);
    for(int i = iterOffset; i < m_offlineMap->operationalLayers()->size(); i++){
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(i));
        connect(fl->featureTable(), &FeatureTable::queryFeaturesCompleted, this, [this, fl](QUuid, Esri::ArcGISRuntime::FeatureQueryResult* fqResult){
            m_fieldNames = fqResult->iterator().features().at(0)->attributes()->attributeNames();
            emit fieldNamesChanged();
            m_tableList = fqResult->iterator().features().at(0)->attributes();
            emit tableListChanged();
            for(int i = 0; i < fqResult->iterator().features().size(); i++){
                QVariantMap newEl;
                AttributeListModel* alm = fqResult->iterator().features().at(i)->attributes();
                for(int x = 0; x < alm->attributeNames().size(); x++){
                    newEl.insert(alm->attributeNames().at(x), alm->attributeValue(alm->attributeNames().at(x)));
                }
                m_attTable->appendElement(newEl);
            }
        });
    }
}*/

//PURPOSE: Listener for feature identification
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::listen_Identify(){
    connect(m_mapView, &MapQuickView::identifyLayerCompleted, this, [this](QUuid, Esri::ArcGISRuntime::IdentifyLayerResult* identifyResult){
        if(!identifyResult){return;}
        QList<Popup*> popups = identifyResult->popups();
        for(int i = 0; i < popups.size(); i++){
            Feature* f = static_cast<Feature*>(popups[i]->geoElement());
            FeatureLayer* fl(nullptr);
            for(int j = iterOffset; j < m_offlineMap->operationalLayers()->size(); j++){
                fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(j));
                fl->clearSelection();
                if(QString::compare(identifyResult->layerContent()->name(), fl->name(), Qt::CaseInsensitive) == 0){
                    fl->selectFeature(f);
                }
            }
            set_puManager(popups[i], fl, f);
        }
    });
}

//PURPOSE: Build or rebuild the attribute table and setup signals for the table
//PARAMETERS: N/A
//RETURN: N/A
/*void Cleanup::buildTable() {
    m_attTable = new AttributeTable();

    connect(m_attTable, &AttributeTable::dataAppended, this, [this](QVariantMap el){ emit dataAppended(el); });
    connect(m_attTable, &AttributeTable::dataRemoved, this, [this](QVariantMap el){ emit dataRemoved(el); });
    connect(m_attTable, &AttributeTable::listCleared, this, [this](){ emit listCleared(); });

    FeatureLayer* fl(nullptr);
    fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(currentTable));
    emit changeTableName(fl->name());
    QueryParameters qp;
    qp.setWhereClause("1=1");//Query for every single feature for the table
    fl->featureTable()->queryFeatures(qp);
}*/

//PURPOSE: Set popup manager anytime it changes
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::set_puManager(Popup* p, FeatureLayer* fl, Feature* f){
    p->popupDefinition()->setTitle("Assign Cleanup Crew");//Set the title
    m_puManager = new PopupManager(p, this);

    for(int i = 0; i < m_puManager->displayedFields()->popupFields().size(); i++){
        PopupField *puField = m_puManager->displayedFields()->popupFields().at(i);
        if(!m_puManager->domain(puField).isEmpty()){
            Error error = m_puManager->updateValue(m_puManager->formattedValue(puField), puField);//TODO
            emit puDataChanged();
            if(!error.isEmpty()){qDebug() << error.message().toUtf8();}
        }
    }

    connect(m_puManager, &PopupManager::errorOccurred, this, [this](Error error){
        if(!error.isEmpty()){
            qDebug(error.message().toUtf8());
        }
    });

    connect(m_puManager, &PopupManager::editingCompleted, this, [this](Error error){
        if(!error.isEmpty()){
            qDebug(error.message().toUtf8());
        }
    });

    fl->featureTable()->updateFeature(f);
    m_puManager->startEditing();
    emit puDataChanged();
}

//PURPOSE: Set sync text for download/upload bar
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::setSyncText(QString str){
    m_syncText = str;
    emit syncTextChanged();
}

//PURPOSE: Start syncing by upload or download (depending on the syncDir enumerated input)
//PARAMETERS: syncDir enumerated input
//RETURN: N/A
void Cleanup::startSyncJob(int syncDir){
    if(m_offlineMapSyncTask->loadStatus() == LoadStatus::Loaded){
        OfflineMapSyncParameters* omsp = new OfflineMapSyncParameters();
        omsp->setRollbackOnFailure(true);
        omsp->setSyncDirection(static_cast<Esri::ArcGISRuntime::SyncDirection>(syncDir));
        OfflineMapSyncJob* omsj = m_offlineMapSyncTask->syncOfflineMap(*omsp);

        if(static_cast<Esri::ArcGISRuntime::SyncDirection>(syncDir) == SyncDirection::Download){
            setSyncText(QString("Downloading"));
        }else if(static_cast<Esri::ArcGISRuntime::SyncDirection>(syncDir) == SyncDirection::Upload){
            setSyncText(QString("Uploading"));
        }else if(static_cast<Esri::ArcGISRuntime::SyncDirection>(syncDir) == SyncDirection::Bidirectional){
            setSyncText(QString("Downloading/Uploading"));
        }else if(static_cast<Esri::ArcGISRuntime::SyncDirection>(syncDir) == SyncDirection::None){
            setSyncText(QString("Error"));
        }

        //SYNCING FINISHED
        connect(omsj, &OfflineMapSyncJob::jobDone, this,
                [this, omsj](){
            if(omsj->result()->hasErrors()){
                qDebug() << omsj->error().message().toUtf8();
                setSyncText(QString("Failure to Sync. Will try again"));
            }else{
                qDebug() << "Sync successful";
                setSyncText(QString("Recently Synced"));
            }
        });

        //PROGRESS FOR SYNCING
        connect(omsj, &OfflineMapSyncJob::progressChanged, this,
                [this, omsj](){
            m_syncProgress = (static_cast<double>(omsj->progress()) / 100.0);
            emit syncProgressChanged();
        });
        omsj->start();
    }
}

//PURPOSE: start syncing up from qml or c++
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::syncUp(){
    startSyncJob(static_cast<int>(SyncDirection::Upload));
}

//PURPOSE: start syncing up and down from qml or c++
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::syncUpAndDown(){
    startSyncJob(static_cast<int>(SyncDirection::Bidirectional));
}

//PURPOSE: start syncing down from qml or c++
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::syncDown(){
    startSyncJob(static_cast<int>(SyncDirection::Download));
}

//PURPOSE: Apply edits made in the editing popup window
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::applyEditing(){
    m_puManager->finishEditing();
    FeatureLayer* fl(nullptr);
    for(int j = iterOffset; j < m_offlineMap->operationalLayers()->size(); j++){
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(j));
        fl->clearSelection();
    }
    emit puDataChanged();
    syncUp();
}

//PURPOSE: Cancel edits made in the editing popup window
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::cancelEditing(){
    m_puManager->cancelEditing();
    FeatureLayer* fl(nullptr);
    for(int j = iterOffset; j < m_offlineMap->operationalLayers()->size(); j++){
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(j));
        fl->clearSelection();
    }
    emit puDataChanged();
}

//PURPOSE: Create signals for offline map
//PARAMETERS: N/A
//RETURN: N/A
void Cleanup::offlineMapTask_Connections(){
    //ERROR
    connect(m_offlineMapTask, &OfflineMapTask::errorOccurred, this,
            [this](Error error){
        qDebug() << error.message();
    });

    //OFFLINE MAP CREATION FROM ONLINE READY
    connect(m_offlineMapTask, &OfflineMapTask::doneLoading, this,
            [this](Error loadError){
        if(!loadError.isEmpty()){qDebug() << loadError.message();}
        qDebug() << "Done loading offline map task";
        Geometry extent = m_map->initialViewpoint().targetGeometry();
        m_offlineMapTask->createDefaultGenerateOfflineMapParameters(extent);
    });

    //CREATE OFFLINE MAP GENERATION PARAMETERS
    connect(m_offlineMapTask, &OfflineMapTask::createDefaultGenerateOfflineMapParametersCompleted, this,
            [this](QUuid, const GenerateOfflineMapParameters& parameters){
        GenerateOfflineMapParameters params = parameters;
        params.setMaxScale(5000.0);
        params.setAttachmentSyncDirection(AttachmentSyncDirection::Upload);
        params.setReturnLayerAttachmentOption(ReturnLayerAttachmentOption::AllLayers);
        params.itemInfo().setTitle(params.itemInfo().title() + " (Central");
        params.setIncludeBasemap(true);
        qDebug() << "Checking whether map with these parameters can be taken offline...";
        m_offlineMapTask->offlineMapCapabilities(params);
        if(QDir(mmpkPath.toUtf8()).exists()){
            QDir(mmpkPath.toUtf8()).removeRecursively();
        }
        QDir().mkdir(mmpkPath.toUtf8());
        GenerateOfflineMapJob* gomj = m_offlineMapTask->generateOfflineMap(params, mmpkPath);

        //CREATING OFFLINE MAP FROM ONLINE FINISHED
        connect(gomj, &GenerateOfflineMapJob::jobDone, this,
                [this, gomj](){
            m_offlineMap = gomj->result()->offlineMap(this);
            /*for(auto e : gomj->result()->layerErrors().keys()){
                qDebug() << "Layer: " << e->name().toUtf8();
                qDebug() << gomj->result()->layerErrors().value(e).message().toUtf8() << "\n";
            }*/

            //LOAD THE OFFLINE MAP
            connect(m_offlineMap, &Map::doneLoading, this,
                    [this](Error loadError){
                if(!loadError.isEmpty()){qDebug() << loadError.message();}
                m_offlineMapSyncTask = new OfflineMapSyncTask(m_offlineMap, this);
                offlineMapSyncTask_Connections();
                m_offlineMapSyncTask->load();
                m_mapView->setMap(m_offlineMap);
                /*for(int i=iterOffset; i<m_offlineMap->operationalLayers()->size(); i++){
                    qDebug() << "Layer: " << m_offlineMap->operationalLayers()->at(i)->name().toUtf8() << "\n";
                }*/
                makeConnections();
            });
            m_offlineMap->load();
            setSyncText(QString("Done"));
        });

        //PROGRESS FOR CREATING OFFLINE MAP FROM ONLINE
        connect(gomj, &GenerateOfflineMapJob::progressChanged, this,
                [this, gomj](){
            setSyncText(QString("Downloading"));
            m_syncProgress = (static_cast<double>(gomj->progress()) / 100.0);
            emit syncProgressChanged();
        });
        gomj->start();
    });

    connect(m_offlineMapTask, &OfflineMapTask::offlineMapCapabilitiesCompleted, this,
            [this] (QUuid, const OfflineMapCapabilities& capabilities)
    {
        if (!capabilities.hasErrors()){
            qDebug() << "This map may be taken offline.";
        }else
            qDebug() << "This map may NOT be taken offline.";
    });
}

//PURPOSE: move to next table (clean up areas or streets)
//PARAMETERS: N/A
//RETURN: N/A
/*void Cleanup::nextTable(){
    if(currentTable < (m_offlineMap->operationalLayers()->size()-1)){
        FeatureLayer* fl(nullptr);
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(currentTable));
        if(fl->name() == "Clean Up Streets" || fl->name() == "Clean Up Areas"){
            m_attTable->clearList();
            currentTable++;
            buildTable();
        }
    }
}*/

//PURPOSE: move to previous table (clean up areas or streets)
//PARAMETERS: N/A
//RETURN: N/A
/*void Cleanup::prevTable(){
    if(currentTable > 0){
        FeatureLayer* fl(nullptr);
        fl = static_cast<FeatureLayer*>(m_offlineMap->operationalLayers()->at(currentTable));
        if(fl->name() == "Clean Up Streets" || fl->name() == "Clean Up Areas"){
            m_attTable->clearList();
            currentTable--;
            buildTable();
        }
    }
}*/

//PURPOSE: Get authentication manager if you need to
//PARAMETERS: N/A
//RETURN: N/A
AuthenticationManager* Cleanup::authenticationManager() const
{
    return AuthenticationManager::instance();
}
