#include "debug_break.h"

#include <sciter_value.h>

SciterValue::SciterValue() :
    m_type(TYPE_UNDEFINED),
    m_valueInt(0)
{
}

SciterValue::~SciterValue()
{
}

SciterValue::SciterValue(bool Value) :
    m_type(TYPE_BOOL),
    m_valueInt(Value)
{
}

SciterValue::SciterValue(int32_t Value) :
    m_type(TYPE_INT32_T),
    m_valueInt(Value)
{
}

SciterValue::SciterValue(std::string Value) :
    m_type(TYPE_STRING),
    m_valueStr(std::move(Value))
{
}

bool SciterValue::isBool(void) const
{
    return m_type == TYPE_BOOL;
}

bool SciterValue::isInt(void) const
{
    return m_type == TYPE_INT32_T;
}

bool SciterValue::isString(void) const
{
    return m_type == TYPE_STRING;
}

bool SciterValue::GetValueBool(void) const
{
    if (m_type != TYPE_BOOL)
    {
        SciterUI::DebugBreak();
        return false;
    }
    return m_valueInt != 0;
}

int32_t SciterValue::GetValueInt(void) const
{
    if (m_type != TYPE_INT32_T)
    {
        SciterUI::DebugBreak();
        return 0;
    }
    return m_valueInt;
}

const std::string & SciterValue::GetValueStr(void) const
{
    if (m_type != TYPE_STRING)
    {
        SciterUI::DebugBreak();
        static const std::string empty;
        return empty;
    }
    return m_valueStr;
}
