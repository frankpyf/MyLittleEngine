#pragma once
namespace renderer {
	class DependencyGraph
	{
	public:
		DependencyGraph() noexcept;
		~DependencyGraph() noexcept = default;

		class Node;

		struct Edge 
		{
			const uint32_t from;
			const uint32_t to;

			Edge(DependencyGraph& graph, Node* from, Node* to)
				:from(from->GetId()), to(to->GetId())
			{
				assert(graph.nodes_[this->from] == from);
				assert(graph.nodes_[this->to] == to);
				graph.Link(this);
			};

			Edge(Edge const&) noexcept = delete;
			Edge& operator=(Edge const&) noexcept = delete;
		};

		class Node
		{
			friend class DependencyGraph;
		public:
			Node(DependencyGraph& graph);
			virtual ~Node() = default;

			Node(Node const&) noexcept = delete;
			Node& operator=(Node const&) noexcept = delete;
			
			virtual char const* GetName() const noexcept { return "base"; };
			
			inline uint32_t GetId() const noexcept { return id_; }
			inline uint8_t GetOutDegree() const noexcept { return out_degree_; };

			bool IsCulled() const noexcept { return out_degree_ == 0; };

			void DontCull() { out_degree_++; };

		private:
			uint8_t out_degree_ = 0;

			uint32_t id_;
		};

		std::vector<Edge*> GetIncomingEdges(Node const* node) const;
		std::vector<Edge*> GetOutgoingEdges(Node const* node) const;

		Node* GetNode(uint32_t id) { return nodes_[id]; };

		void Cull();

		void Clear();

	private:
		void RegisterNode(Node* in_node);
		void Link(Edge* edge);

		std::vector<Edge*> edges_;
		std::vector<Node*> nodes_;
	};
}

