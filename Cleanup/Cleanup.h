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

#ifndef CLEANUP_H
#define CLEANUP_H

namespace Esri
{
namespace ArcGISRuntime
{
class Map;
class MapQuickView;
class MobileMapPackage;
class LegendInfoListModel;
class PopupManager;
class Popup;
class OfflineMapTask;
class AuthenticationManager;
class FeatureLayer;
class Feature;
class OfflineMapSyncTask;
class AttributeListModel;
}
}

#include <QQuickItem>
#include "attributetable.h"

class Cleanup : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(bool mapDrawing READ mapDrawing NOTIFY mapDrawStatusChanged)
    Q_PROPERTY(Esri::ArcGISRuntime::LegendInfoListModel* legendInfoListModel READ legendInfoListModel NOTIFY legendInfoListModelChanged)
    Q_PROPERTY(Esri::ArcGISRuntime::PopupManager* puManager READ puManager NOTIFY puDataChanged)
    Q_PROPERTY(bool queryFailure READ queryFailure NOTIFY queryFailureChanged)
    Q_PROPERTY(Esri::ArcGISRuntime::AuthenticationManager* authenticationManager READ authenticationManager CONSTANT)
    Q_PROPERTY(double syncProgress READ syncProgress NOTIFY syncProgressChanged)
    Q_PROPERTY(QString syncText READ syncText NOTIFY syncTextChanged)

    /*
    Q_PROPERTY(QStringList fieldNames READ fieldNames NOTIFY fieldNamesChanged)
    Q_PROPERTY(Esri::ArcGISRuntime::AttributeListModel* tableList READ tableList NOTIFY tableListChanged)
    */

public:
    Cleanup(QQuickItem* parent = nullptr);
    ~Cleanup();

    void componentComplete() Q_DECL_OVERRIDE;
    static void init();

    void listen_Query();
    void listen_Identify();

    /*
    void listen_Table();
    */

    void offlineMapTask_Connections();
    void offlineMapSyncTask_Connections();
    void makeConnections();
    void startSyncJob(int syncDir);
    void set_puManager(Esri::ArcGISRuntime::Popup* p, Esri::ArcGISRuntime::FeatureLayer* fl, Esri::ArcGISRuntime::Feature* f);
    void setSyncText(QString str);

    /*
    void buildTable();
    */

    Q_INVOKABLE void applyEditing();
    Q_INVOKABLE void cancelEditing();
    Q_INVOKABLE void runQuery(const QString& qInput, const QString& trigger);
    Q_INVOKABLE void syncUp();
    Q_INVOKABLE void syncDown();
    Q_INVOKABLE void syncUpAndDown();

    /*
    Q_INVOKABLE void nextTable();
    Q_INVOKABLE void prevTable();
    */

signals:
    void mapDrawStatusChanged();
    void legendInfoListModelChanged();
    void puDataChanged();
    void queryFailureChanged();
    void syncProgressChanged();
    void syncTextChanged();
    void fqResultAdded(QVariant result);
    void clearResults();

    /*
    void tableListChanged();
    void fieldNamesChanged();
    void dataAppended(QVariantMap row);
    void dataRemoved(QVariantMap row);
    void listCleared();
    void changeTableName(const QString &name);
    */

private:
    bool mapDrawing() const { return m_mapDrawing; }
    Esri::ArcGISRuntime::LegendInfoListModel* legendInfoListModel() const { return m_legendInfoListModel; }
    Esri::ArcGISRuntime::PopupManager* puManager() const { return m_puManager; }
    Esri::ArcGISRuntime::AuthenticationManager* authenticationManager() const;
    bool queryFailure() const { return m_queryFailure; }
    double syncProgress() const { return m_syncProgress; }
    QString syncText() const { return m_syncText; }

    /*
    Esri::ArcGISRuntime::AttributeListModel* tableList() const { return m_tableList; }
    QStringList fieldNames() const { return m_fieldNames; }
    */

private:
    Esri::ArcGISRuntime::Map*                   m_map;
    Esri::ArcGISRuntime::Map*                   m_offlineMap;
    Esri::ArcGISRuntime::MapQuickView*          m_mapView;
    Esri::ArcGISRuntime::MobileMapPackage*      m_mobileMapPackage;
    Esri::ArcGISRuntime::LegendInfoListModel*   m_legendInfoListModel;
    Esri::ArcGISRuntime::PopupManager*          m_puManager;
    Esri::ArcGISRuntime::OfflineMapTask*        m_offlineMapTask;
    Esri::ArcGISRuntime::OfflineMapSyncTask*    m_offlineMapSyncTask;
    bool m_mapDrawing;
    bool m_queryFailure;
    int failureCnt;
    QString mmpkPath;
    QString mmpkPath2;
    int iterOffset;
    int currentTable;
    int syncInterval;
    double m_syncProgress;
    QString m_syncText;

    /*
    Esri::ArcGISRuntime::AttributeListModel* m_tableList;
    QStringList m_fieldNames;
    AttributeTable* m_attTable;
    */
};

#endif // CLEANUP_H
