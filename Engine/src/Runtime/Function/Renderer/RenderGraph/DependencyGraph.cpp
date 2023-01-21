#include "mlepch.h"
#include "DependencyGraph.h"

namespace renderer {
	DependencyGraph::DependencyGraph() noexcept
	{
		nodes_.reserve(8);
		edges_.reserve(16);
	}

	void DependencyGraph::Cull()
	{
		// update in degree for each node
		for (Edge* const edge : edges_) {
			Node* node = nodes_[edge->from];
			node->out_degree_++;
		}

		// Topo Sort
		std::vector<Node*> stack{};
		stack.reserve(nodes_.size());
		
		for (auto node : nodes_)
		{
			if (node->out_degree_ == 0)
			{
				stack.push_back(node);
			}
		}

		while (!stack.empty())
		{
			Node* const node = stack.back();
			stack.pop_back();

			for (auto edge: GetIncomingEdges(node))
			{
				Node* linked_node = GetNode(edge->from);
				if (--linked_node->out_degree_ == 0) {
					stack.push_back(linked_node);
				}
			}
		}
	}

	void DependencyGraph::Clear()
	{
		nodes_.clear();
		edges_.clear();
	}

	std::vector<DependencyGraph::Edge*> DependencyGraph::GetIncomingEdges(Node const* node) const
	{
		std::vector<Edge*> result;
		uint32_t const id = node->GetId();
		std::copy_if(edges_.begin(), edges_.end(),
			std::back_insert_iterator<std::vector<Edge*>>(result),
			[id](auto edge) { return edge->to == id; });

		return result;
	}

	std::vector<DependencyGraph::Edge*> DependencyGraph::GetOutgoingEdges(Node const* node) const
	{
		std::vector<Edge*> result;
		uint32_t const id = node->GetId();
		std::copy_if(edges_.begin(), edges_.end(),
			std::back_insert_iterator<std::vector<Edge*>>(result),
			[id](auto edge) { return edge->from == id; });

		return result;
	}

	void DependencyGraph::RegisterNode(Node* in_node)
	{
		nodes_.push_back(in_node);
	}

	void DependencyGraph::Link(Edge* edge)
	{
		edges_.push_back(edge);
	}

	//-----------------------------Node-----------------------------
	DependencyGraph::Node::Node(DependencyGraph& graph)
		:id_(graph.nodes_.size())
	{
		graph.RegisterNode(this);
	}
}