#include "clay.hpp"

namespace clay {


//
// ValueHolder constructor and destructor
//

ValueHolder::ValueHolder(TypePtr type)
    : Object(VALUE_HOLDER), type(type)
{
    this->buf = (char *)calloc(1, typeSize(type));
}

ValueHolder::~ValueHolder()
{
    // FIXME: call clay 'destroy'
    free(this->buf);
}



//
// objectEquals
//

// FIXME: this doesn't handle arbitrary values (need to call clay)
bool _objectValueEquals(ObjectPtr a, ObjectPtr b)
{
    // at this point pointer identity should already have been checked by objectEquals

    int aKind = a->objKind, bKind = b->objKind;
    if (aKind != bKind)
        return false;

    switch (aKind) {

    case IDENTIFIER : {
        Identifier *a1 = (Identifier *)a.ptr();
        Identifier *b1 = (Identifier *)b.ptr();
        return a1->str == b1->str;
    }

    case VALUE_HOLDER : {
        ValueHolder *a1 = (ValueHolder *)a.ptr();
        ValueHolder *b1 = (ValueHolder *)b.ptr();
        if (a1->type != b1->type)
            return false;
        size_t n = typeSize(a1->type);
        return memcmp(a1->buf, b1->buf, n) == 0;
    }

    case PVALUE : {
        PValue *a1 = (PValue *)a.ptr();
        PValue *b1 = (PValue *)b.ptr();
        return a1->data == b1->data;
    }

    case MODULE_HOLDER : {
        ModuleHolder *a1 = (ModuleHolder*)a.ptr();
        ModuleHolder *b1 = (ModuleHolder*)b.ptr();
        return a1->module == b1->module;
    }

    default :
        return false;
    }
}



//
// objectHash
//

static int identityHash(Object *a)
{
    size_t v = (size_t)a;
    return (int)v;
}

// FIXME: this doesn't handle arbitrary values (need to call clay)
int objectHash(ObjectPtr a)
{
    switch (a->objKind) {

    case IDENTIFIER : {
        Identifier *b = (Identifier *)a.ptr();
        int h = 0;
        for (unsigned i = 0; i < b->str.size(); ++i)
            h += b->str[i];
        return h;
    }

    case VALUE_HOLDER : {
        ValueHolder *b = (ValueHolder *)a.ptr();
        // TODO: call clay 'hash'
        int h = 0;
        size_t n = typeSize(b->type);
        for (int i = 0; i < n; ++i)
            h += b->buf[i];
        h = h*11 + identityHash(b->type.ptr());
        return h;
    }

    case PVALUE : {
        PValue *b = (PValue *)a.ptr();
        int h = identityHash(b->data.type.ptr());
        h *= b->data.isTemp ? 11 : 23;
        return h;
    }

/* XXX
    case MODULE_HOLDER : {
        ModuleHolder *b = (ModuleHolder*)a.ptr();
        return identityHash(b->module);
    }
*/

    default : {
        return identityHash(a.ptr());
    }

    }
}



//
// ObjectTable
//

ObjectPtr &ObjectTable::lookup(const vector<ObjectPtr> &key)
{
    if (this->buckets.empty())
        this->buckets.resize(16);
    if (this->buckets.size() < 2*this->size)
        rehash();
    int h = objectVectorHash(key);
    h &= (buckets.size() - 1);
    vector<ObjectTableNode> &bucket = buckets[h];
    for (unsigned i = 0; i < bucket.size(); ++i) {
        if (objectVectorEquals(key, bucket[i].key))
            return bucket[i].value;
    }
    bucket.push_back(ObjectTableNode(key, NULL));
    ++this->size;
    return bucket.back().value;
}

void ObjectTable::rehash()
{
    vector< vector<ObjectTableNode> > newBuckets;
    newBuckets.resize(4 * this->buckets.size());

    for (unsigned i = 0; i < this->buckets.size(); ++i) {
        vector<ObjectTableNode> &bucket = this->buckets[i];
        for (unsigned j = 0; j < bucket.size(); ++j) {
            int h = objectVectorHash(bucket[j].key);
            h &= (newBuckets.size() - 1);
            newBuckets[h].push_back(bucket[j]);
        }
    }

    this->buckets = newBuckets;
}

}
