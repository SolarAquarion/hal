#include "netlist/gate_library/gate_type/gate_type_ff.h"

gate_type_ff::gate_type_ff(const std::string& name) : gate_type(name)
{
    m_base_type = base_type::ff;
    m_ascending = true;
}

bool gate_type_ff::doCompare(const gate_type& other) const
{
    bool equal             = false;
    const gate_type_ff* gt = dynamic_cast<const gate_type_ff*>(&other);

    if (gt)
    {
        equal = m_next_state_f == gt->get_next_state_function();
        equal &= m_clock_f == gt->get_clock_function();
        equal &= m_set_f == gt->get_set_function();
        equal &= m_reset_f == gt->get_reset_function();
        equal &= m_state_pins == gt->get_state_output_pins();
        equal &= m_inverted_state_pins == gt->get_inverted_state_output_pins();
        equal &= m_special_behavior == gt->get_special_behavior();
        equal = m_data_category == gt->get_data_category();
        equal &= m_data_identifier == gt->get_data_identifier();
        equal &= m_ascending == gt->is_ascending_order();
    }

    return equal;
}

void gate_type_ff::set_next_state_function(const boolean_function& next_state_f)
{
    m_next_state_f = next_state_f;
}

void gate_type_ff::set_clock_function(const boolean_function& clock_f)
{
    m_clock_f = clock_f;
}

void gate_type_ff::set_set_function(const boolean_function& set_f)
{
    m_set_f = set_f;
}

void gate_type_ff::set_reset_function(const boolean_function& reset_f)
{
    m_reset_f = reset_f;
}

void gate_type_ff::add_state_output_pin(std::string output_pin_name)
{
    m_state_pins.insert(output_pin_name);
}

void gate_type_ff::add_inverted_state_output_pin(std::string output_pin_name)
{
    m_inverted_state_pins.insert(output_pin_name);
}

void gate_type_ff::set_special_behavior(special_behavior sb1, special_behavior sb2)
{
    m_special_behavior = {sb1, sb2};
}

void gate_type_ff::set_data_category(const std::string& category)
{
    m_data_category = category;
}

void gate_type_ff::set_data_identifier(const std::string& identifier)
{
    m_data_identifier = identifier;
}

void gate_type_ff::set_data_ascending_order(bool ascending)
{
    m_ascending = ascending;
}

boolean_function gate_type_ff::get_next_state_function() const
{
    return m_next_state_f;
}

boolean_function gate_type_ff::get_clock_function() const
{
    return m_clock_f;
}

boolean_function gate_type_ff::get_set_function() const
{
    return m_set_f;
}

boolean_function gate_type_ff::get_reset_function() const
{
    return m_reset_f;
}

std::unordered_set<std::string> gate_type_ff::get_state_output_pins() const
{
    return m_state_pins;
}

std::unordered_set<std::string> gate_type_ff::get_inverted_state_output_pins() const
{
    return m_inverted_state_pins;
}

std::pair<gate_type::special_behavior, gate_type::special_behavior> gate_type_ff::get_special_behavior() const
{
    return m_special_behavior;
}

std::string gate_type_ff::get_data_category() const
{
    return m_data_category;
}

std::string gate_type_ff::get_data_identifier() const
{
    return m_data_identifier;
}

bool gate_type_ff::is_ascending_order() const
{
    return m_ascending;
}
