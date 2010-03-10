#include "uavdataobject.h"

UAVDataObject::UAVDataObject(quint32 objID, quint32 instID, bool isSingleInst, QString& name, quint32 numBytes):
        UAVObject(objID, instID, isSingleInst, name, numBytes)
{
    data = new quint8[numBytes]; // create data buffer, object fields are stored there as a packed structure
    mobj = NULL;
}

void UAVDataObject::initialize(QList<UAVObjectField*> fields, Metadata& mdata)
{
    UAVMetaObject* obj = new UAVMetaObject(objID+1, name.append("Meta"), mdata, this);
    initialize(fields, obj);
}

void UAVDataObject::initialize(QList<UAVObjectField*> fields, UAVMetaObject* mobj)
{
    // Initialize fields and create data buffer
    this->mobj = mobj;
    this->fields = fields;
    quint32 offset = 0;
    for (int n = 0; n < fields.length(); ++n)
    {
        fields.value(n)->initialize(data, offset, this);
        offset += fields.value(n)->getNumBytes();
        connect(fields.value(n), SIGNAL(fieldUpdated(UAVObjectField*)), this, SLOT(fieldUpdated(UAVObjectField*)));
    }
}

void UAVDataObject::fieldUpdated(UAVObjectField* field)
{
    emit objectUpdated(this, false); // trigger object updated event
}

qint32 UAVDataObject::getNumFields()
{
    return fields.count();
}

QList<UAVObjectField*> UAVDataObject::getFields()
{
    return fields;
}

UAVObjectField* UAVDataObject::getField(QString& name)
{
    // Look for field
    for (int n = 0; n < fields.length(); ++n)
    {
        if (name.compare(fields.value(n)->getName()) == 0)
        {
            return fields.value(n);
        }
    }
    // If this point is reached then the field was not found
    return NULL;
}

qint32 UAVDataObject::pack(quint8* dataOut)
{
    QMutexLocker locker(mutex);
    memcpy(dataOut, data, numBytes);
    return numBytes;
}

qint32 UAVDataObject::unpack(const quint8* dataIn)
{
    QMutexLocker locker(mutex);
    memcpy(data, dataIn, numBytes);
    emit objectUpdated(this, true); // trigger object updated event
    return numBytes;
}

void UAVDataObject::setMetadata(const Metadata& mdata)
{
    mobj->setData(mdata);
}

UAVObject::Metadata UAVDataObject::getMetadata(void)
{
    return mobj->getData();
}

UAVMetaObject* UAVDataObject::getMetaObject()
{
    return mobj;
}
