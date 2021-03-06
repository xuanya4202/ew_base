#ifndef __EWA_MATH_DEF_H__
#define __EWA_MATH_DEF_H__

#include "ewa_base/math/tiny_cpx.h"
#include "ewa_base/math/tiny_vec.h"
#include "ewa_base/math/tiny_mat.h"
#include "ewa_base/math/tiny_box.h"

EW_ENTER


typedef tiny_vec<int32_t,3> vec3i;
typedef tiny_vec<double,3> vec3d;
typedef tiny_vec<double,2> vec2d;

typedef tiny_box<int32_t,3> box3i;
typedef tiny_box<double,3> box3d;

typedef type_mat<double,4,4> mat4d;


typedef tiny_storage<String,3> vec3s;
typedef tiny_storage<String,2> vec2s;

class box3s
{
public:
	vec3s lo,hi;
};

template<typename T>
class DLLIMPEXP_EWA_BASE numeric_trait_floating
{
public:
	static double nan_value();
	static double min_value();
	static double max_value();
};

template<typename T>
class DLLIMPEXP_EWA_BASE numeric_trait_integer
{
public:
	static double min_value();
	static double max_value();
};

template<typename T>
class DLLIMPEXP_EWA_BASE numeric_trait : public tl::meta_if<tl::integer_type::id<T>::value!=-1, numeric_trait_integer<T>, numeric_trait_floating<T> >::type
{
public:

};



EW_LEAVE
#endif
