#include <gtest/gtest.h>
#include "defs.h"

// ============================================================
// Fixture: ElementStatesTest
// ============================================================
class ElementStatesTest : public ::testing::Test
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

// Event watcher for ElementStatesChanged events
class StatesChangeWatcher : public EventFilter
{
public:
	StatesChangeWatcher(Element* element)
		: m_element(element)
	{
		if (m_element)
			m_element->RegisterEventFilter(this);
	}

	~StatesChangeWatcher()
	{
		if (m_element)
			m_element->UnRegisterEventFilter(this);
	}

	int changeCount = 0;
	Element* lastChangedElement = nullptr;

	bool Filter(EventTarget* target, Event* e) override
	{
		if (e->type == Type::ElementStatesChanged)
		{
			changeCount++;
			auto event = static_cast<ElementStatesChangedEvent*>(e);
			lastChangedElement = event->element;
		}
		return false;
	}

private:
	Element* m_element = nullptr;
};

// ============================================================
// Default state tests
// ============================================================
TEST_F(ElementStatesTest, DefaultStateIsEnabled)
{
	EXPECT_TRUE(root->States()->IsEnabled());
	EXPECT_FALSE(root->States()->IsHovered());
	EXPECT_FALSE(root->States()->IsPressed());
	EXPECT_FALSE(root->States()->IsFocused());
	EXPECT_FALSE(root->States()->IsSelected());
}

TEST_F(ElementStatesTest, GetStatePropsReturnsEmptyForDefaultState)
{
	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"");
}

// ============================================================
// Set / Get individual states
// ============================================================
TEST_F(ElementStatesTest, SetEnabled)
{
	StatesChangeWatcher watcher(root);

	root->States()->SetEnabled(false);
	EXPECT_FALSE(root->States()->IsEnabled());
	EXPECT_EQ(watcher.changeCount, 1);
	EXPECT_EQ(watcher.lastChangedElement, root);

	root->States()->SetEnabled(true);
	EXPECT_TRUE(root->States()->IsEnabled());
	EXPECT_EQ(watcher.changeCount, 2);
}

TEST_F(ElementStatesTest, SetHovered)
{
	StatesChangeWatcher watcher(root);

	root->States()->SetHovered(true);
	EXPECT_TRUE(root->States()->IsHovered());
	EXPECT_EQ(watcher.changeCount, 1);

	root->States()->SetHovered(false);
	EXPECT_FALSE(root->States()->IsHovered());
	EXPECT_EQ(watcher.changeCount, 2);
}

TEST_F(ElementStatesTest, SetPressed)
{
	StatesChangeWatcher watcher(root);

	root->States()->SetPressed(true);
	EXPECT_TRUE(root->States()->IsPressed());
	EXPECT_EQ(watcher.changeCount, 1);

	root->States()->SetPressed(false);
	EXPECT_FALSE(root->States()->IsPressed());
	EXPECT_EQ(watcher.changeCount, 2);
}

TEST_F(ElementStatesTest, SetFocused)
{
	StatesChangeWatcher watcher(root);

	root->States()->SetFocused(true);
	EXPECT_TRUE(root->States()->IsFocused());
	EXPECT_EQ(watcher.changeCount, 1);

	root->States()->SetFocused(false);
	EXPECT_FALSE(root->States()->IsFocused());
	EXPECT_EQ(watcher.changeCount, 2);
}

TEST_F(ElementStatesTest, SetSelected)
{
	StatesChangeWatcher watcher(root);

	root->States()->SetSelected(true);
	EXPECT_TRUE(root->States()->IsSelected());
	EXPECT_EQ(watcher.changeCount, 1);

	root->States()->SetSelected(false);
	EXPECT_FALSE(root->States()->IsSelected());
	EXPECT_EQ(watcher.changeCount, 2);
}

// ============================================================
// Setting same state twice is no-op
// ============================================================
TEST_F(ElementStatesTest, SettingSameStateIsNoOp)
{
	StatesChangeWatcher watcher(root);

	// Already enabled by default
	root->States()->SetEnabled(true);
	EXPECT_EQ(watcher.changeCount, 0);

	root->States()->SetHovered(false);
	EXPECT_EQ(watcher.changeCount, 0);
}

// ============================================================
// GetStateProps priority
// ============================================================
TEST_F(ElementStatesTest, GetStatePropsReturnsDisabledFirst)
{
	root->States()->SetEnabled(false);
	root->States()->SetHovered(true);
	root->States()->SetPressed(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"disabled");
}

TEST_F(ElementStatesTest, GetStatePropsReturnsPressed)
{
	root->States()->SetPressed(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"pressed");
}

TEST_F(ElementStatesTest, GetStatePropsReturnsHovered)
{
	root->States()->SetHovered(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"hovered");
}

TEST_F(ElementStatesTest, GetStatePropsReturnsSelected)
{
	root->States()->SetSelected(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"selected");
}

TEST_F(ElementStatesTest, GetStatePropsReturnsFocused)
{
	root->States()->SetFocused(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"focused");
}

// ============================================================
// GetStateProps priority: disabled > pressed > hovered > selected > focused
// ============================================================
TEST_F(ElementStatesTest, GetStatePropsPressedOverHovered)
{
	root->States()->SetHovered(true);
	root->States()->SetPressed(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"pressed");
}

TEST_F(ElementStatesTest, GetStatePropsHoveredOverSelected)
{
	root->States()->SetSelected(true);
	root->States()->SetHovered(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"hovered");
}

TEST_F(ElementStatesTest, GetStatePropsSelectedOverFocused)
{
	root->States()->SetFocused(true);
	root->States()->SetSelected(true);

	EXPECT_TRUE(std::u8string_view(root->States()->GetStateProps()) == u8"selected");
}

// ============================================================
// Enabled inheritance from parent
// ============================================================
TEST_F(ElementStatesTest, ChildEnabledInheritsFromParent)
{
	Element* child = new Element(root);
	Element* grandchild = new Element(child);

	EXPECT_TRUE(child->States()->IsEnabled());
	EXPECT_TRUE(grandchild->States()->IsEnabled());

	root->States()->SetEnabled(false);

	EXPECT_FALSE(child->States()->IsEnabled());
	EXPECT_FALSE(grandchild->States()->IsEnabled());

	root->States()->SetEnabled(true);

	EXPECT_TRUE(child->States()->IsEnabled());
	EXPECT_TRUE(grandchild->States()->IsEnabled());
}

TEST_F(ElementStatesTest, ChildOwnEnabledOverridesInherited)
{
	Element* child = new Element(root);

	// Child explicitly disabled
	child->States()->SetEnabled(false);
	EXPECT_FALSE(child->States()->IsEnabled());
	EXPECT_TRUE(root->States()->IsEnabled());

	// Parent also disabled
	root->States()->SetEnabled(false);
	EXPECT_FALSE(child->States()->IsEnabled());
	EXPECT_FALSE(root->States()->IsEnabled());

	// Parent re-enabled, child still disabled
	root->States()->SetEnabled(true);
	EXPECT_FALSE(child->States()->IsEnabled());
	EXPECT_TRUE(root->States()->IsEnabled());

	// Child re-enabled
	child->States()->SetEnabled(true);
	EXPECT_TRUE(child->States()->IsEnabled());
}

// ============================================================
// Auto state change from Enter/Leave events
// ============================================================
TEST_F(ElementStatesTest, EnterEventSetsHovered)
{
	Event enterEvent(Type::Enter);
	root->FireEvent(&enterEvent);

	EXPECT_TRUE(root->States()->IsHovered());
}

TEST_F(ElementStatesTest, LeaveEventClearsHovered)
{
	// First hover
	root->States()->SetHovered(true);
	EXPECT_TRUE(root->States()->IsHovered());

	Event leaveEvent(Type::Leave);
	root->FireEvent(&leaveEvent);

	EXPECT_FALSE(root->States()->IsHovered());
}
