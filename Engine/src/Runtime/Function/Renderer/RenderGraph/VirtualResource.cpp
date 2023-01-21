#include "mlepch.h"
#include "VirtualResource.h"
#include "Nodes.h"

namespace renderer {
	void VirtualResource::Connect(DependencyGraph& dg, ResourceNode* resource, PassNode* pass)
	{
		/*DependencyGraph::Edge* edge;
		if (edge == nullptr)
		{
			edge = new DependencyGraph::Edge(dg, (DependencyGraph::Node*)resource, (DependencyGraph::Node*)pass);
			resource->SetOutgoingEdge(edge);
		}*/

		DependencyGraph::Edge* edge = new DependencyGraph::Edge(dg, (DependencyGraph::Node*)resource, (DependencyGraph::Node*)pass);
		resource->SetOutgoingEdge(edge);
	}

	void VirtualResource::Connect(DependencyGraph& dg, PassNode* pass, ResourceNode* resource)
	{
		/*DependencyGraph::Edge* edge;
		if (edge == nullptr)
		{
			edge = new DependencyGraph::Edge(dg, (DependencyGraph::Node*)pass, (DependencyGraph::Node*)resource);
			resource->SetIncomingEdge(edge);
		}*/
		DependencyGraph::Edge* edge = new DependencyGraph::Edge(dg, (DependencyGraph::Node*)pass, (DependencyGraph::Node*)resource);
		resource->SetIncomingEdge(edge);
	}

	void VirtualResource::NeedByPass(PassNode* node)
	{
		ref_count_++;

		first_ = first_ ? first_ : node;
		last_ = node;
	}
}