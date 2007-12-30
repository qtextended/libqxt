#ifndef QxtBdbHash_H_kpasd
#define QxtBdbHash_H_kpasd

#include "qxtbdb.h"
#include <QBuffer>
#include <QDataStream>
#include <QVariant>
#include <qxtsharedprivate.h>


template<class KEY, class VAL>
class QxtBdbHash
{
public:
    QxtBdbHash(QString file);

    void clear();
    bool contains ( const KEY & key ) const;
    bool remove ( const KEY & key );
    bool insert(KEY k, VAL v);
    const VAL value (  const KEY & key  ) const;
    const VAL operator[] ( const KEY & key ) const;

    bool flush();

private:
    int meta_id_key;
    int meta_id_val;
    QxtSharedPrivate<QxtBdb> qxt_d;
};



template<class KEY, class VAL>
QxtBdbHash<KEY,VAL>::QxtBdbHash(QString file)
{
    qxt_d=new QxtBdb();
    qxt_d().open(file,QxtBdb::CreateDatabase);

    ///this will fail to compile if KEY or VALUE are not declared metatypes. Good.
    meta_id_key = qMetaTypeId<KEY>(); 
    meta_id_val = qMetaTypeId<VAL>(); 
}



template<class KEY, class VAL>
void QxtBdbHash<KEY,VAL>::clear()
{
    u_int32_t x;
    qxt_d().db->truncate(qxt_d().db,NULL, &x,0);

}


template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::contains ( const KEY & k ) const
{

    BerkeleyDB::DBT key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));

    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();

    return (qxt_d().db->exists(qxt_d().db, NULL, &key,0)==0);
}

template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::remove ( const KEY & k )
{

    BerkeleyDB::DBT key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));

    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();


    return (qxt_d().db->del(qxt_d().db, NULL, &key,0)==0);
}



template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::insert(KEY k, VAL v)
{
    QByteArray d_key, d_value;

    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_val, &v));
        buffer.close();
    }

    BerkeleyDB::DBT key, value;
    /* Zero out the DBTs before using them. */
    memset(&key, 0,  sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));

    key.data = d_key.data();
    key.size = d_key.size();

    value.data = d_value.data();
    value.size = d_value.size();


    int ret = qxt_d().db->put(qxt_d().db, NULL, &key, &value, 0);
    return (ret==0);

}



template<class KEY, class VAL>
const VAL QxtBdbHash<KEY,VAL>::value ( const KEY & k  ) const
{
    BerkeleyDB::DBT key, data;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));
    memset(&data, 0, sizeof(BerkeleyDB::DBT));


    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::save (s,meta_id_key, &k));
        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();

    data.data = 0;
    data.ulen = 0;
    data.flags =0;
    qxt_d().db->get(qxt_d().db, NULL, &key, &data, 0);


    QByteArray  d_value((const char*) data.data, data.size );


    /**
    FIXME: Db deletes the record when i close the database. So i cant even delete it now.
           in other words, it leaks...
    */
//     delete data.data;

    VAL v;
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        Q_ASSERT(QMetaType::load (s,meta_id_val, &v));
        buffer.close();
    }
    return v;
}

template<class KEY, class VAL>
const VAL QxtBdbHash<KEY,VAL>::operator[] (  const KEY & key  ) const
{
    return value(key);
}


template<class KEY, class VAL>
bool QxtBdbHash<KEY,VAL>::flush()
{
    return qxt_d().flush();
}


#endif