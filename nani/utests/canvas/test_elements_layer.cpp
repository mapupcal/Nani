#include <gtest/gtest.h>
#include "defs.h"

// ============================================================
// Fixture: ElementsLayerTest
// ============================================================
class ElementsLayerTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		root = new Element(nullptr);
	}

	void TearDown() override
	{
		delete root;
		root = nullptr;
	}

	Element* root = nullptr;
};

// Event watcher to count ElementAdd / ElementRemove events on a specific parent
class LayerEventWatcher : public EventFilter
{
public:
	LayerEventWatcher(Element* parent)
		: m_parent(parent)
	{
		if (m_parent)
			m_parent->RegisterEventFilter(this);
	}

	~LayerEventWatcher()
	{
		if (m_parent)
			m_parent->UnRegisterEventFilter(this);
	}

	int addCount = 0;
	int removeCount = 0;

	bool Filter(EventTarget* target, Event* e) override
	{
		if (e->type == Type::ElementAdd) addCount++;
		if (e->type == Type::ElementRemove) removeCount++;
		return false;
	}

private:
	target_ptr<Element> m_parent;
};

TEST_F(ElementsLayerTest, AddElementToLayer)
{
	LayerEventWatcher watcher(root);
	Element* child = new Element(nullptr);

	root->Layer()->AddElement(child);

	EXPECT_EQ(child->Parent(), root);
	EXPECT_EQ(root->Children().size(), 1u);
	EXPECT_EQ(watcher.addCount, 1);
}

TEST_F(ElementsLayerTest, AddNullDoesNothing)
{
	LayerEventWatcher watcher(root);
	root->Layer()->AddElement(nullptr);

	EXPECT_EQ(root->Children().size(), 0u);
	EXPECT_EQ(watcher.addCount, 0);
}

TEST_F(ElementsLayerTest, AddAlreadyOwnedChildIsNoOp)
{
	Element* child = new Element(root);
	LayerEventWatcher watcher(root);

	root->Layer()->AddElement(child);

	EXPECT_EQ(root->Children().size(), 1u);
	// Should not fire another add event
	EXPECT_EQ(watcher.addCount, 0);
}

TEST_F(ElementsLayerTest, AddElementMovesFromOldParent)
{
	LayerEventWatcher rootWatcher(root);

	Element* otherRoot = new Element(nullptr);
	LayerEventWatcher otherWatcher(otherRoot);

	Element* child = new Element(otherRoot);
	EXPECT_EQ(otherRoot->Children().size(), 1u);
	EXPECT_EQ(root->Children().size(), 0u);

	// Move child from otherRoot to root
	root->Layer()->AddElement(child);

	EXPECT_EQ(child->Parent(), root);
	EXPECT_EQ(root->Children().size(), 1u);
	EXPECT_EQ(otherRoot->Children().size(), 0u);
	EXPECT_EQ(rootWatcher.addCount, 1);
	EXPECT_EQ(otherWatcher.removeCount, 1);

	delete otherRoot;
}

TEST_F(ElementsLayerTest, RemoveElementFromLayer)
{
	Element* child = new Element(root);
	LayerEventWatcher watcher(root);
	EXPECT_EQ(root->Children().size(), 1u);

	root->Layer()->RemoveElement(child);

	EXPECT_EQ(child->Parent(), nullptr);
	EXPECT_EQ(root->Children().size(), 0u);
	EXPECT_EQ(watcher.removeCount, 1);

	delete child;
}

TEST_F(ElementsLayerTest, RemoveNullDoesNothing)
{
	LayerEventWatcher watcher(root);
	root->Layer()->RemoveElement(nullptr);

	EXPECT_EQ(watcher.removeCount, 0);
}

TEST_F(ElementsLayerTest, RemoveNonChildDoesNothing)
{
	Element* otherRoot = new Element(nullptr);
	Element* foreignChild = new Element(otherRoot);
	LayerEventWatcher watcher(root);

	root->Layer()->RemoveElement(foreignChild);

	EXPECT_EQ(watcher.removeCount, 0);
	EXPECT_EQ(foreignChild->Parent(), otherRoot);

	delete otherRoot;
}

TEST_F(ElementsLayerTest, RemoveIsNoOpDuringDestruction)
{
	// When a layer is being destroyed, it deletes all children.
	// RemoveElement should be a no-op during destruction to avoid re-entry issues.
	Element* parent = new Element(nullptr);
	Element* child = new Element(parent);

	// Simulate the layer destruction path: deleting parent destroys its layer
	// which sets m_bDestroying = true and deletes all children.
	// We test by deleting the parent directly — RemoveElement is called from
	// child's destructor on the parent's layer, which should be no-op since
	// m_bDestroying is already true.
	EXPECT_NO_FATAL_FAILURE({
		delete parent;
	});
}

TEST_F(ElementsLayerTest, EnumElementsReturnsCorrectCount)
{
	Element* a = new Element(root);
	Element* b = new Element(root);
	Element* c = new Element(root);

	auto elements = root->Layer()->Elements();
	EXPECT_EQ(elements.size(), 3u);
}

TEST_F(ElementsLayerTest, ElementAddEventContainsCorrectElement)
{
	struct AddVerifier : public EventFilter
	{
		Element* expected = nullptr;
		bool verified = false;

		bool Filter(EventTarget* target, Event* e) override
		{
			if (e->type == Type::ElementAdd)
			{
				auto event = static_cast<ElementModifyEvent*>(e);
				verified = (event->element == expected);
			}
			return false;
		}
	};

	AddVerifier verifier;
	verifier.expected = nullptr; // will be the new element
	root->RegisterEventFilter(&verifier);

	Element* child = new Element(nullptr);
	verifier.expected = child;
	root->Layer()->AddElement(child);

	EXPECT_TRUE(verifier.verified);

	root->UnRegisterEventFilter(&verifier);
}

TEST_F(ElementsLayerTest, ElementRemoveEventContainsCorrectElement)
{
	struct RemoveVerifier : public EventFilter
	{
		Element* expected = nullptr;
		bool verified = false;

		bool Filter(EventTarget* target, Event* e) override
		{
			if (e->type == Type::ElementRemove)
			{
				auto event = static_cast<ElementModifyEvent*>(e);
				verified = (event->element == expected);
			}
			return false;
		}
	};

	Element* child = new Element(root);
	RemoveVerifier verifier;
	verifier.expected = child;
	root->RegisterEventFilter(&verifier);

	root->Layer()->RemoveElement(child);
	EXPECT_TRUE(verifier.verified);

	root->UnRegisterEventFilter(&verifier);
	delete child;
}
