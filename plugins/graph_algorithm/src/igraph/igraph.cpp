#include "core/log.h"
#include "core/plugin_manager.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "plugin_graph_algorithm.h"

#include <igraph/igraph.h>
#include <tuple>

namespace hal
{
    std::tuple<igraph_t, std::map<int, Gate*>> plugin_graph_algorithm::get_igraph_directed(Netlist* const nl)
    {
        igraph_t graph;

        // count all edges, remember in HAL one net(edge) has multiple sinks
        u32 edge_counter = 0;
        for (const auto& net : nl->get_nets())
        {
            Gate* src_gate = net->get_source().get_gate();
            std::vector<Gate*> dst_gates;

            auto dst_gates_endpoints = net->get_destinations();

            for (const auto& dst_gate_endpoint : dst_gates_endpoints)
            {
                dst_gates.push_back(dst_gate_endpoint.get_gate());
            }

            // if gate has no src --> add exactly one dummy node
            if (!src_gate)
            {
                edge_counter += dst_gates.size();
            }
            // if gate has no dsts --> add dummy node
            else if (dst_gates.size() == 0)
            {
                edge_counter++;
            }
            // default mode
            else
            {
                edge_counter += dst_gates.size();
            }
        }

        log_debug("graph_algorithm", "nets: {}, edge_counter: {}", nl->get_nets().size(), edge_counter);

        // initialize edge vector
        igraph_vector_t edges;
        igraph_vector_init(&edges, 2 * edge_counter);

        // we need dummy gates for input/outputs
        u32 dummy_gate_counter   = nl->get_gates().size() - 1;
        u32 edge_vertice_counter = 0;

        for (const auto& net : nl->get_nets())
        {
            Gate* src_gate = net->get_source().get_gate();
            std::vector<Gate*> dst_gates;

            auto dst_gates_endpoints = net->get_destinations();

            for (const auto& dst_gate_endpoint : dst_gates_endpoints)
            {
                dst_gates.push_back(dst_gate_endpoint.get_gate());
            }

            // if gate has no src --> add exactly one dummy node
            if (src_gate == nullptr)
            {
                u32 dummy_gate = ++dummy_gate_counter;
                for (const auto& dst_gate : dst_gates)
                {
                    VECTOR(edges)[edge_vertice_counter++] = dummy_gate;
                    VECTOR(edges)[edge_vertice_counter++] = dst_gate->get_id() - 1;

                    log_debug("graph_algorithm", "input_gate: {} --> {}: {}", dummy_gate, dst_gate->get_id() - 1, dst_gate->get_name().c_str());
                }
            }
            // if gate has no dsts --> add dummy node
            else if (dst_gates.size() == 0)
            {
                VECTOR(edges)[edge_vertice_counter++] = src_gate->get_id() - 1;
                VECTOR(edges)[edge_vertice_counter++] = ++dummy_gate_counter;

                log_debug("graph_algorithm", "{}: {} --> {} output\n", src_gate->get_name().c_str(), src_gate->get_id() - 1, dummy_gate_counter);
            }
            // default mode
            else
            {
                for (const auto& dst_gate : dst_gates)
                {
                    VECTOR(edges)[edge_vertice_counter++] = src_gate->get_id() - 1;
                    VECTOR(edges)[edge_vertice_counter++] = dst_gate->get_id() - 1;

                    log_debug("graph_algorithm", "{}: {} --> {}: {}", src_gate->get_name().c_str(), src_gate->get_id() - 1, dst_gate->get_id() - 1, dst_gate->get_name().c_str());
                }
            }
        }

        igraph_create(&graph, &edges, 0, IGRAPH_DIRECTED);

        // map with vertice id to hal-gate
        std::map<int, Gate*> vertice_to_gate;
        for (auto const& gate : nl->get_gates())
        {
            vertice_to_gate[gate->get_id() - 1] = gate;
        }

        return std::make_tuple(graph, vertice_to_gate);
    }

    std::map<int, std::set<Gate*>> plugin_graph_algorithm::get_memberships_for_hal(igraph_t graph, igraph_vector_t membership, std::map<int, Gate*> vertex_to_gate)
    {
        // map back to HAL structures
        int vertices_num = (int)igraph_vcount(&graph);
        std::map<int, std::set<Gate*>> community_sets;

        for (int i = 0; i < vertices_num; i++)
        {
            auto gate = vertex_to_gate[i];
            if (gate == nullptr)
                continue;
            community_sets[VECTOR(membership)[i]].insert(gate);
        }
        return community_sets;
    }
}    // namespace hal
