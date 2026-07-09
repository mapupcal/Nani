#include <gtest/gtest.h>
#include "defs.h"

class ElementTest : public ::testing::Test
{
protected:
	void SetUp() override
	{

	}

	void TearDown() override
	{

	}
};


TEST_F(ElementTest, ElementsTreeAndLifeCycle)
{
	{
		Element* root = new Element(nullptr);
		EXPECT_EQ(root->Parent(), nullptr);
		EXPECT_EQ(root->Children().size(), 0);

		Element* child1 = new Element(root);
		EXPECT_EQ(child1->Parent(), root);
		EXPECT_EQ(root->Parent(), nullptr);
		EXPECT_EQ(root->Children().size(), 1);

		delete root;
	}

	{
		Element* root = new Element(nullptr);
		Element* child1 = new Element(root);
		EXPECT_EQ(root->Children().size(), 1);
		delete child1;
		EXPECT_EQ(root->Children().size(), 0);
		delete root;
	}

	{
		Element* root = new Element(nullptr);
		Element* root2 = new Element(nullptr);
		Element* child1 = new Element(root2);
		EXPECT_EQ(root->Children().size(), 0);
		EXPECT_EQ(root2->Children().size(), 1);
		root->Layer()->AddElement(child1);
		EXPECT_EQ(root->Children().size(), 1);
		EXPECT_EQ(root2->Children().size(), 0);

		delete child1;
		delete root2;
		delete root;
	}

	{
		Element* root = new Element(nullptr);
		Element* child1 = new Element(root);
		Element* child2 = new Element(root);
		EXPECT_EQ(root->Children().size(), 2);
		EXPECT_EQ(child1->Children().size(), 0);
		EXPECT_EQ(child2->Children().size(), 0);

		root->Layer()->AddElement(child1);
		root->Layer()->AddElement(child2);
		EXPECT_EQ(root->Children().size(), 2);


		child1->Layer()->AddElement(child2);
		EXPECT_EQ(root->Children().size(), 1);
		EXPECT_EQ(child1->Children().size(), 1);

		delete child1;
		EXPECT_EQ(root->Children().size(), 0);
		delete root;
	}
}

TEST_F(ElementTest, VisibilityFlagsCanBeCleared)
{
	Element root(nullptr);

	root.Visibility()->SetHidden(true);
	EXPECT_TRUE(root.Visibility()->IsHidden());
	EXPECT_FALSE(root.Visibility()->IsVisible());

	root.Visibility()->SetHidden(false);
	EXPECT_FALSE(root.Visibility()->IsHidden());
	EXPECT_TRUE(root.Visibility()->IsVisible());

	root.Visibility()->SetCollapsed(true);
	EXPECT_TRUE(root.Visibility()->IsCollapsed());
	EXPECT_FALSE(root.Visibility()->IsVisible());

	root.Visibility()->SetCollapsed(false);
	EXPECT_FALSE(root.Visibility()->IsCollapsed());
	EXPECT_TRUE(root.Visibility()->IsVisible());
}

TEST_F(ElementTest, ChildInheritsParentStyles)
{
	auto styles = std::make_shared<Styles>();
	Element root(nullptr);
	Element child(&root);

	EXPECT_EQ(root.GetStyles(), nullptr);
	EXPECT_EQ(child.GetStyles(), nullptr);

	root.SetStyles(styles);
	EXPECT_EQ(child.GetStyles(), styles.get());
}

TEST_F(ElementTest, StyleClassAcceptsStringView)
{
	Element root(nullptr);

	root.SetStyleClass(u8"button");
	EXPECT_TRUE(root.StyleClass() == u8"button");
}

TEST_F(ElementTest, ChildrenInheritParentHiddenVisibilityAndDisableState)
{
	{
		Element root(nullptr);
		Element child(&root);
		Element child2(&root);
		root.Visibility()->SetHidden(true);
		EXPECT_TRUE(child.Visibility()->IsHidden());
		EXPECT_TRUE(child2.Visibility()->IsHidden());

		root.Visibility()->SetHidden(false);
		EXPECT_FALSE(child.Visibility()->IsHidden());
		EXPECT_FALSE(child2.Visibility()->IsHidden());

		root.States()->SetEnabled(false);
		EXPECT_FALSE(child.States()->IsEnabled());
		EXPECT_FALSE(child2.States()->IsEnabled());

		root.States()->SetEnabled(true);
		EXPECT_TRUE(child.States()->IsEnabled());
		EXPECT_TRUE(child2.States()->IsEnabled());
	}

	{
		Element root(nullptr);
		Element father(&root);
		Element child(&father);
		root.Visibility()->SetHidden(true);
		EXPECT_TRUE(father.Visibility()->IsHidden());
		EXPECT_TRUE(child.Visibility()->IsHidden());

		root.Visibility()->SetHidden(false);
		EXPECT_FALSE(father.Visibility()->IsHidden());
		EXPECT_FALSE(child.Visibility()->IsHidden());

		root.States()->SetEnabled(false);
		EXPECT_FALSE(father.States()->IsEnabled());
		EXPECT_FALSE(child.States()->IsEnabled());

		root.States()->SetEnabled(true);
		EXPECT_TRUE(father.States()->IsEnabled());
		EXPECT_TRUE(child.States()->IsEnabled());

		father.Visibility()->SetHidden(true);
		EXPECT_FALSE(root.Visibility()->IsHidden());
		EXPECT_TRUE(father.Visibility()->IsHidden());
		EXPECT_TRUE(child.Visibility()->IsHidden());

		father.Visibility()->SetHidden(false);
		EXPECT_FALSE(root.Visibility()->IsHidden());
		EXPECT_FALSE(father.Visibility()->IsHidden());
		EXPECT_FALSE(child.Visibility()->IsHidden());

		father.States()->SetEnabled(false);
		EXPECT_TRUE(root.States()->IsEnabled());
		EXPECT_FALSE(father.States()->IsEnabled());
		EXPECT_FALSE(child.States()->IsEnabled());

		father.States()->SetEnabled(true);
		EXPECT_TRUE(root.States()->IsEnabled());
		EXPECT_TRUE(father.States()->IsEnabled());
		EXPECT_TRUE(child.States()->IsEnabled());
	}
}

TEST_F(ElementTest, ElementStatesSetGet)
{
	Element root(nullptr);

	EXPECT_TRUE(root.States()->IsEnabled());
	EXPECT_FALSE(root.States()->IsHovered());
	EXPECT_FALSE(root.States()->IsPressed());
	EXPECT_FALSE(root.States()->IsFocused());
	EXPECT_FALSE(root.States()->IsSelected());

	root.States()->SetHovered(true);
	root.States()->SetPressed(true);
	root.States()->SetFocused(true);
	root.States()->SetSelected(true);

	EXPECT_TRUE(root.States()->IsHovered());
	EXPECT_TRUE(root.States()->IsPressed());
	EXPECT_TRUE(root.States()->IsFocused());
	EXPECT_TRUE(root.States()->IsSelected());

	root.States()->SetHovered(false);
	root.States()->SetPressed(false);
	root.States()->SetFocused(false);
	root.States()->SetSelected(false);

	EXPECT_FALSE(root.States()->IsHovered());
	EXPECT_FALSE(root.States()->IsPressed());
	EXPECT_FALSE(root.States()->IsFocused());
	EXPECT_FALSE(root.States()->IsSelected());
}

TEST_F(ElementTest, ElementVisibilityCollapsed)
{
	Element root(nullptr);

	EXPECT_TRUE(root.Visibility()->IsVisible());

	root.Visibility()->SetCollapsed(true);
	EXPECT_TRUE(root.Visibility()->IsCollapsed());
	EXPECT_FALSE(root.Visibility()->IsVisible());

	root.Visibility()->SetCollapsed(false);
	EXPECT_FALSE(root.Visibility()->IsCollapsed());
	EXPECT_TRUE(root.Visibility()->IsVisible());
}

TEST_F(ElementTest, ElementCollapsedDoesNotPropagateToChildren)
{
	Element root(nullptr);
	Element child(&root);

	root.Visibility()->SetCollapsed(true);
	EXPECT_TRUE(root.Visibility()->IsCollapsed());
	EXPECT_FALSE(child.Visibility()->IsCollapsed());

	root.Visibility()->SetCollapsed(false);
	EXPECT_FALSE(root.Visibility()->IsCollapsed());
}

TEST_F(ElementTest, ElementStyleClassDefaultIsElement)
{
	Element root(nullptr);
	Element child(&root);

	EXPECT_TRUE(child.StyleClass() == u8"Element");
}

TEST_F(ElementTest, ElementGetStylesReturnsNullWithoutAssignment)
{
	Element root(nullptr);
	Element child(&root);

	EXPECT_EQ(child.GetStyles(), nullptr);
}

TEST_F(ElementTest, ElementGetStylesFromAncestor)
{
	auto styles = std::make_shared<Styles>();
	Element root(nullptr);
	Element child(&root);
	Element grandchild(&child);

	root.SetStyles(styles);

	EXPECT_EQ(child.GetStyles(), styles.get());
	EXPECT_EQ(grandchild.GetStyles(), styles.get());
}

TEST_F(ElementTest, ElementStatesInheritsDisabled)
{
	Element root(nullptr);
	Element child1(&root);
	Element child2(&root);

	EXPECT_TRUE(child1.States()->IsEnabled());
	EXPECT_TRUE(child2.States()->IsEnabled());

	root.States()->SetEnabled(false);

	EXPECT_FALSE(child1.States()->IsEnabled());
	EXPECT_FALSE(child2.States()->IsEnabled());
}

TEST_F(ElementTest, ElementCreateVisualIsNonNull)
{
	Element root(nullptr);

	auto visual = root.CreateVisual(nullptr, nullptr);
	EXPECT_NE(visual, nullptr);
}
