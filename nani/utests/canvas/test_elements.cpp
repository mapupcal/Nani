#include <gtest/gtest.h>
#include "canvas/elements/element.h"
#include "canvas/elements/elements_layer.h"
#include "canvas/elements/element_visibility.h"
#include "canvas/elements/styles.h"

using namespace nani::canvas;
using namespace nani::canvas::basic;
using namespace nani::canvas::elements;

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


TEST_F(ElementTest, WindowProperties)
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
	EXPECT_EQ(root.StyleClass(), u8"button");
}

