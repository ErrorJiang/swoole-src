/*
  +----------------------------------------------------------------------+
  | Swoole                                                               |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
  +----------------------------------------------------------------------+
*/

#pragma once

#include "php_swoole.h"
extern "C"
{
#include "ext/standard/php_var.h"
}

#include <unordered_map>
#include <string>

#define MAX_ARGC        20
#define VAR_DUMP_LEVEL  10

using namespace std;

namespace PHP
{
class Variant
{
public:
    Variant()
    {
        init();
        ZVAL_NULL(&val);
    }
    Variant(long v)
    {
        init();
        ZVAL_LONG(&val, v);
    }
    Variant(int v)
    {
        init();
        ZVAL_LONG(&val, (long )v);
    }
    Variant(const char *str)
    {
        init();
        ZVAL_STRING(&val, str);
    }
    Variant(const char *str, size_t len)
    {
        init();
        ZVAL_STRINGL(&val, str, len);
    }
    Variant(string &str)
    {
        init();
        ZVAL_STRINGL(&val, str.c_str(), str.length());
    }
    Variant(double v)
    {
        init();
        ZVAL_DOUBLE(&val, v);
    }
    Variant(float v)
    {
        init();
        ZVAL_DOUBLE(&val, (double )v);
    }
    Variant(bool v)
    {
        init();
        ZVAL_BOOL(&val, v);
    }
    Variant(zval *v)
    {
        reference = false;
        ref_val = NULL;
        memcpy(&val, v, sizeof(zval));
        zval_add_ref(&val);
    }
    Variant(zval *v, bool ref)
    {
        ref_val = v;
        reference = ref;
    }
    ~Variant()
    {
        if (!reference)
        {
            zval_ptr_dtor(&val);
        }
    }
    void operator =(int v)
    {
        ZVAL_LONG(&val, (long )v);
    }
    void operator =(long v)
    {
        ZVAL_LONG(&val, v);
    }
    void operator =(string &str)
    {
        ZVAL_STRINGL(ptr(), str.c_str(), str.length());
    }
    void operator =(const char *str)
    {
        ZVAL_STRING(ptr(), str);
    }
    void operator =(double v)
    {
        ZVAL_DOUBLE(ptr(), v);
    }
    void operator =(float v)
    {
        ZVAL_DOUBLE(ptr(), (double )v);
    }
    void operator =(bool v)
    {
        ZVAL_BOOL(ptr(), v);
    }
    zval *ptr(void)
    {
        if (reference)
        {
            return ref_val;
        }
        else
        {
            return &val;
        }
    }
    int type()
    {
        return Z_TYPE(val);
    }
    bool isString()
    {
        return Z_TYPE(val) == IS_STRING;
    }
    bool isArray()
    {
        return Z_TYPE(val) == IS_ARRAY;
    }
    bool isObject()
    {
        return Z_TYPE(val) == IS_OBJECT;
    }
    bool isInt()
    {
        return Z_TYPE(val) == IS_LONG;
    }
    bool isFloat()
    {
        return Z_TYPE(val) == IS_DOUBLE;
    }
    bool isBool()
    {
        return Z_TYPE(val) == IS_TRUE || Z_TYPE(val) == IS_FALSE;
    }
    bool isNull()
    {
        return Z_TYPE(val) == IS_NULL;
    }
    bool isResource()
    {
        return Z_TYPE(val) == IS_RESOURCE;
    }
    bool isReference()
    {
        return Z_TYPE(val) == IS_REFERENCE;
    }
    string toString()
    {
        return string(Z_STRVAL_P(&val), Z_STRLEN_P(&val));
    }
    char* toCString()
    {
        return Z_STRVAL_P(&val);
    }
    long toInt()
    {
        return Z_LVAL_P(&val);
    }
    double toFloat()
    {
        return Z_DVAL_P(&val);
    }
    bool toBool()
    {
        return Z_BVAL_P(&val) == 1;
    }
protected:
    bool reference;
    zval *ref_val;
    zval val;
    void init()
    {
        reference = false;
        ref_val = NULL;
        memset(&val, 0, sizeof(val));
    }
};

class ArrayIterator
{
public:
    ArrayIterator(Bucket *p)
    {
        _ptr = p;
        _key = _ptr->key;
        _val = &_ptr->val;
        _index = _ptr->h;
    }
    void operator ++(int i)
    {
        while (1)
        {
            _ptr++;
            _val = &_ptr->val;
            if (_val && Z_TYPE_P(_val) == IS_INDIRECT)
            {
                _val = Z_INDIRECT_P(_val);
            }
            if (UNEXPECTED(Z_TYPE_P(_val) == IS_UNDEF))
            {
                continue;
            }
            if (_key)
            {
                _key = _ptr->key;
                _index = 0;
            }
            else
            {
                _index = _ptr->h;
                _key = NULL;
            }
            break;
        }
    }
    bool operator !=(ArrayIterator b)
    {
        return b.ptr() != _ptr;
    }
    Variant key()
    {
        if (_key)
        {
            return Variant(_key->val, _key->len);
        }
        else
        {
            return Variant((long) _index);
        }
    }
    Variant value()
    {
        return Variant(_val);
    }
    Bucket *ptr()
    {
        return _ptr;
    }
private:
    zval *_val;
    zend_string *_key;
    Bucket *_ptr;
    zend_ulong _index;
};

class Array: public Variant
{
public:
    Array() :
            Variant()
    {
        array_init(&val);
    }
    Array(zval *v) :
            Variant(v)
    {

    }
    Array(Variant &v) :
            Variant()
    {
        memcpy(&val, v.ptr(), sizeof(val));
        zval_add_ref(&val);
    }
    void append(Variant &v)
    {
        add_next_index_zval(&val, v.ptr());
    }
    void append(Variant v)
    {
        add_next_index_zval(&val, v.ptr());
    }
    void append(const char *str)
    {
        add_next_index_string(&val, str);
    }
    void append(string &str)
    {
        add_next_index_stringl(&val, str.c_str(), str.length());
    }
    void append(long v)
    {
        add_next_index_long(&val, v);
    }
    void append(int v)
    {
        add_next_index_long(&val, (long) v);
    }
    void append(bool v)
    {
        add_next_index_bool(&val, v ? 1 : 0);
    }
    void append(double v)
    {
        add_next_index_double(&val, (double) v);
    }
    void append(float v)
    {
        add_next_index_double(&val, (double) v);
    }
    void append(void *v)
    {
        add_next_index_null(&val);
    }
    void append(Array &v)
    {
        zval_add_ref(v.ptr());
        add_next_index_zval(&val, v.ptr());
    }
    //------------------------------------
    void set(const char *key, Variant &v)
    {
        add_assoc_zval(&val, key, v.ptr());
    }
    void set(const char *key, long v)
    {
        add_assoc_long(&val, key, v);
    }
    void set(const char *key, const char *v)
    {
        add_assoc_string(&val, key, (char * )v);
    }
    void set(const char *key, string &v)
    {
        add_assoc_stringl(&val, key, (char* )v.c_str(), v.length());
    }
    void set(const char *key, double v)
    {
        add_assoc_double(&val, key, v);
    }
    void set(const char *key, float v)
    {
        add_assoc_double(&val, key, (double ) v);
    }
    void set(const char *key, bool v)
    {
        add_assoc_bool(&val, key, v ? 1 : 0);
    }
    void set(int i, Variant &v)
    {
        add_index_zval(&val, (zend_ulong) i, v.ptr());
    }
    Variant operator [](int i) const
    {
        zval *ret = zend_hash_index_find(Z_ARRVAL(val), (zend_ulong) i);
        return Variant(ret);
    }
    Variant operator [](const char *key) const
    {
        zval *ret = zend_hash_str_find(Z_ARRVAL(val), key, strlen(key));
        return Variant(ret);
    }
    bool remove(const char *key)
    {
        zend_string *_key = zend_string_init(key, strlen(key), 0);
        bool ret = zend_hash_del(Z_ARRVAL(val), _key) == SUCCESS;
        zend_string_free(_key);
        return ret;
    }
    void clean()
    {
        zend_hash_clean(Z_ARRVAL(val));
    }
    bool exists(const char *key)
    {
        zend_string *_key = zend_string_init(key, strlen(key), 0);
        bool ret = zend_hash_exists(Z_ARRVAL(val), _key) == SUCCESS;
        zend_string_free(_key);
        return ret;
    }
    ArrayIterator begin()
    {
        return ArrayIterator(Z_ARRVAL(val)->arData);
    }
    ArrayIterator end()
    {
        return ArrayIterator(Z_ARRVAL(val)->arData + Z_ARRVAL(val)->nNumUsed);
    }
    size_t count()
    {
        return Z_ARRVAL(val)->nNumOfElements;
    }
};

static inline Variant _call(zval *object, zval *func, Array &args)
{
    Variant retval = false;
    if (args.count() > MAX_ARGC)
    {
        return retval;
    }
    zval params[MAX_ARGC];
    for (int i = 0; i < args.count(); i++)
    {
        ZVAL_COPY_VALUE(&params[i], args[i].ptr());
    }
    zval _retval;
    if (call_user_function(EG(function_table), object, func, &_retval, args.count(), params) == 0)
    {
        retval = Variant(&_retval);
    }
    return retval;
}

static inline Variant _call(zval *object, zval *func)
{
    Variant retval = false;
    zval params[0];
    zval _retval;
    if (call_user_function(EG(function_table), object, func, &_retval, 0, params) == 0)
    {
        retval = Variant(&_retval);
    }
    return retval;
}

Variant call(Variant &func, Array &args)
{
    return _call(NULL, func.ptr(), args);
}

Variant call(const char *func, Array &args)
{
    Variant _func(func);
    return _call(NULL, _func.ptr(), args);
}

void var_dump(Array &array)
{
    php_var_dump(array.ptr(), VAR_DUMP_LEVEL);
}

class Object: public Variant
{
public:
    Object(Variant &v) :
            Variant()
    {
        memcpy(&val, v.ptr(), sizeof(val));
        zval_add_ref(&val);
    }
    Object(zval *v) :
            Variant(v)
    {

    }
    Object() :
            Variant()
    {

    }
    Variant call(Variant &func, Array &args)
    {
        return _call(ptr(), func.ptr(), args);
    }
    Variant call(const char *func, Array &args)
    {
        Variant _func(func);
        return _call(ptr(), _func.ptr(), args);
    }
    /**
     * call php function with 0 params.
     */
    Variant call(Variant &func)
    {
        return _call(ptr(), func.ptr());
    }
    Variant call(const char *func)
    {
        Variant _func(func);
        return _call(ptr(), _func.ptr());
    }
    Variant get(const char *name)
    {
        Variant retval;
        zval rv;
        zval *member_p = zend_read_property(Z_OBJCE_P(&val), &val, name, strlen(name), 0, &rv);
        if (member_p != &rv)
        {
            ZVAL_COPY(retval.ptr(), member_p);
        }
        else
        {
            ZVAL_COPY_VALUE(retval.ptr(), member_p);
        }
        return retval;
    }

    void set(const char *name, Variant &v)
    {
        zend_update_property(Z_OBJCE_P(&val), &val, name, strlen(name), v.ptr());
    }

    void set(const char *name, Array &v)
    {
        zend_update_property(Z_OBJCE_P(&val), &val, name, strlen(name), v.ptr());
    }

    void set(const char *name, string &v)
    {
        zend_update_property_stringl(Z_OBJCE_P(&val), &val, name, strlen(name), v.c_str(), v.length());
    }
    void set(const char *name, char *v)
    {
        zend_update_property_string(Z_OBJCE_P(&val), &val, name, strlen(name), v);
    }
    void set(const char *name, long v)
    {
        zend_update_property_long(Z_OBJCE_P(&val), &val, name, strlen(name), v);
    }
    void set(const char *name, double v)
    {
        zend_update_property_double(Z_OBJCE_P(&val), &val, name, strlen(name), v);
    }
    void set(const char *name, float v)
    {
        zend_update_property_double(Z_OBJCE_P(&val), &val, name, strlen(name), (double) v);
    }
    void set(const char *name, bool v)
    {
        zend_update_property_bool(Z_OBJCE_P(&val), &val, name, strlen(name), v ? 1 : 0);
    }
};

Object create(const char *name, Array &args)
{
    zend_string *class_name = zend_string_init(name, strlen(name), 0);
    Object object;

    zend_class_entry *ce = zend_lookup_class(class_name);
    zend_string_free(class_name);
    if (ce == NULL)
    {
        swoole_php_error(E_WARNING, "class '%s' is undefined.", name);
        return object;
    }
    zval zobject;
    if (object_init_ex(&zobject, ce) == FAILURE)
    {
        return object;
    }
    object = Object(&zobject);
    object.call("__construct", args);
    return object;
}

Object create(const char *name)
{
    zend_string *class_name = zend_string_init(name, strlen(name), 0);
    Object object;

    zend_class_entry *ce = zend_lookup_class(class_name);
    zend_string_free(class_name);
    if (ce == NULL)
    {
        swoole_php_error(E_WARNING, "class '%s' is undefined.", name);
        return object;
    }
    zval zobject;
    if (object_init_ex(&zobject, ce) == FAILURE)
    {
        return object;
    }
    object = Object(&zobject);
    return object;
}

#define function(f) #f, f
typedef Variant (*function_t)(Array &);
static unordered_map<string, function_t> function_map;

void _call_function(zend_execute_data *data, zval *return_value)
{
    const char *name = data->func->common.function_name->val;
    function_t func = function_map[name];
    Array args;

    zval *param_ptr = ZEND_CALL_ARG(EG(current_execute_data), 1);
    int arg_count = ZEND_CALL_NUM_ARGS(EG(current_execute_data));

    while (arg_count-- > 0)
    {
        args.append(Variant(param_ptr, true));
        param_ptr++;
    }
    Variant retval = func(args);
    ZVAL_COPY_VALUE(return_value, retval.ptr());
    return;
}

void registerFunction(const char *name, function_t func)
{
    zend_function_entry functions[] = {
        {name, _call_function, NULL, (uint32_t) (sizeof(void*) / sizeof(struct _zend_internal_arg_info) - 1), 0 },
        {NULL, NULL, NULL,}
    };
    if (zend_register_functions(NULL, functions, NULL, MODULE_PERSISTENT) == SUCCESS)
    {
        function_map[name] = func;
    }
}

}
