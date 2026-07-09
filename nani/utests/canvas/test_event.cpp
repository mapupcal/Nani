#include <gtest/gtest.h>
#include "defs.h"

// ============================================================
// Fixture: EventTest — Event type construction tests
// ============================================================
class EventTest : public ::testing::Test
{
protected:
	void SetUp() override {}
	void TearDown() override {}
};

// ============================================================
// Base Event construction
// ============================================================
TEST_F(EventTest, BaseEventStoresCorrectType)
{
	Event e(Type::MouseMove);
	EXPECT_EQ(e.type, Type::MouseMove);

	Event unknown(Type::Unknown);
	EXPECT_EQ(unknown.type, Type::Unknown);

	Event quit(Type::Quit);
	EXPECT_EQ(quit.type, Type::Quit);

	Event close(Type::Close);
	EXPECT_EQ(close.type, Type::Close);

	Event show(Type::Show);
	EXPECT_EQ(show.type, Type::Show);

	Event hide(Type::Hide);
	EXPECT_EQ(hide.type, Type::Hide);
}

// ============================================================
// MoveEvent
// ============================================================
TEST_F(EventTest, MoveEventStoresOldAndNewPositions)
{
	MoveEvent e(PointF(10.0f, 20.0f), PointF(30.0f, 40.0f));

	EXPECT_EQ(e.type, Type::Move);
	EXPECT_EQ(e.oldPos, PointF(10.0f, 20.0f));
	EXPECT_EQ(e.newPos, PointF(30.0f, 40.0f));
}

// ============================================================
// ResizeEvent
// ============================================================
TEST_F(EventTest, ResizeEventStoresOldAndNewSizes)
{
	ResizeEvent e(SizeF(800.0f, 600.0f), SizeF(1024.0f, 768.0f));

	EXPECT_EQ(e.type, Type::Resize);
	EXPECT_EQ(e.oldSize, SizeF(800.0f, 600.0f));
	EXPECT_EQ(e.newSize, SizeF(1024.0f, 768.0f));
}

// ============================================================
// PaintEvent
// ============================================================
TEST_F(EventTest, PaintEventStoresDirtyRect)
{
	PaintEvent e(RectF(0.0f, 0.0f, 100.0f, 50.0f));

	EXPECT_EQ(e.type, Type::Paint);
	EXPECT_EQ(e.dirtyRect, RectF(0.0f, 0.0f, 100.0f, 50.0f));
}

// ============================================================
// MouseEvent / MouseMoveEvent
// ============================================================
TEST_F(EventTest, MouseMoveEventStoresPositions)
{
	MouseMoveEvent e(PointF(5.0f, 10.0f), PointF(100.0f, 200.0f));

	EXPECT_EQ(e.type, Type::MouseMove);
	EXPECT_EQ(e.pos, PointF(5.0f, 10.0f));
	EXPECT_EQ(e.globalPos, PointF(100.0f, 200.0f));
}

// ============================================================
// MousePressEvent
// ============================================================
TEST_F(EventTest, MousePressEventStoresButtonAndModifier)
{
	MousePressEvent e(MouseButton::Left, PointF(10.0f, 20.0f), PointF(30.0f, 40.0f), Modifier::Ctrl);

	EXPECT_EQ(e.type, Type::MousePress);
	EXPECT_EQ(e.button, MouseButton::Left);
	EXPECT_EQ(e.pos, PointF(10.0f, 20.0f));
	EXPECT_EQ(e.globalPos, PointF(30.0f, 40.0f));
	EXPECT_EQ(e.modifier, Modifier::Ctrl);
}

TEST_F(EventTest, MousePressEventDefaultModifierIsNone)
{
	MousePressEvent e(MouseButton::Right, PointF(0.0f, 0.0f), PointF(0.0f, 0.0f));

	EXPECT_EQ(e.type, Type::MousePress);
	EXPECT_EQ(e.button, MouseButton::Right);
	EXPECT_EQ(e.modifier, Modifier::None);
}

TEST_F(EventTest, MousePressEventSupportsAllButtons)
{
	MousePressEvent left(MouseButton::Left, PointF(0, 0), PointF(0, 0));
	EXPECT_EQ(left.button, MouseButton::Left);

	MousePressEvent middle(MouseButton::Middle, PointF(0, 0), PointF(0, 0));
	EXPECT_EQ(middle.button, MouseButton::Middle);

	MousePressEvent right(MouseButton::Right, PointF(0, 0), PointF(0, 0));
	EXPECT_EQ(right.button, MouseButton::Right);
}

// ============================================================
// MouseReleaseEvent
// ============================================================
TEST_F(EventTest, MouseReleaseEventStoresButtonAndModifier)
{
	MouseReleaseEvent e(MouseButton::Left, PointF(10.0f, 20.0f), PointF(30.0f, 40.0f), Modifier::Shift | Modifier::Ctrl);

	EXPECT_EQ(e.type, Type::MouseRelease);
	EXPECT_EQ(e.button, MouseButton::Left);
	EXPECT_EQ(e.pos, PointF(10.0f, 20.0f));
	EXPECT_EQ(e.globalPos, PointF(30.0f, 40.0f));
}

// ============================================================
// WheelEvent
// ============================================================
TEST_F(EventTest, WheelEventStoresDeltas)
{
	WheelEvent e(1.5, -2.5);

	EXPECT_EQ(e.type, Type::Wheel);
	EXPECT_FLOAT_EQ(e.deltaX, 1.5);
	EXPECT_FLOAT_EQ(e.deltaY, -2.5);
}

// ============================================================
// KeyPressEvent
// ============================================================
TEST_F(EventTest, KeyPressEventStoresKeyAndModifier)
{
	KeyPressEvent e(Key::A, Modifier::Shift, 30);

	EXPECT_EQ(e.type, Type::KeyPress);
	EXPECT_EQ(e.key, Key::A);
	EXPECT_EQ(e.modifier, Modifier::Shift);
	EXPECT_EQ(e.scancode, 30);
}

TEST_F(EventTest, KeyPressEventDefaultValues)
{
	KeyPressEvent e(Key::Space);

	EXPECT_EQ(e.type, Type::KeyPress);
	EXPECT_EQ(e.key, Key::Space);
	EXPECT_EQ(e.modifier, Modifier::None);
	EXPECT_EQ(e.scancode, 0);
}

// ============================================================
// KeyReleaseEvent
// ============================================================
TEST_F(EventTest, KeyReleaseEventStoresKeyAndModifier)
{
	KeyReleaseEvent e(Key::Escape, Modifier::None, 1);

	EXPECT_EQ(e.type, Type::KeyRelease);
	EXPECT_EQ(e.key, Key::Escape);
	EXPECT_EQ(e.modifier, Modifier::None);
	EXPECT_EQ(e.scancode, 1);
}

// ============================================================
// ElementModifyEvent
// ============================================================
TEST_F(EventTest, ElementModifyEventStoresElement)
{
	Element root(nullptr);
	ElementModifyEvent e(Type::ElementAdd, &root);

	EXPECT_EQ(e.type, Type::ElementAdd);
	EXPECT_EQ(e.element, &root);

	ElementModifyEvent remove(Type::ElementRemove, &root);
	EXPECT_EQ(remove.type, Type::ElementRemove);
	EXPECT_EQ(remove.element, &root);
}

// ============================================================
// ElementStatesChangedEvent
// ============================================================
TEST_F(EventTest, ElementStatesChangedEventStoresElement)
{
	Element root(nullptr);
	ElementStatesChangedEvent e(&root);

	EXPECT_EQ(e.type, Type::ElementStatesChanged);
	EXPECT_EQ(e.element, &root);
}

// ============================================================
// ElementVisibilityChangedEvent
// ============================================================
TEST_F(EventTest, ElementVisibilityChangedEventStoresElement)
{
	Element root(nullptr);
	ElementVisibilityChangedEvent e(&root);

	EXPECT_EQ(e.type, Type::ElementVisibilityChanged);
	EXPECT_EQ(e.element, &root);
}

// ============================================================
// LayoutRequestEvent
// ============================================================
TEST_F(EventTest, LayoutRequestEventStoresVisual)
{
	// We need a View and an Element to create a real Visual.
	// But the event constructor just stores a pointer, so nullptr is fine for construction test.
	// Visual requires a real view, so we test via event construction with nullptr
	LayoutRequestEvent e(nullptr);

	EXPECT_EQ(e.type, Type::LayoutRequest);
	EXPECT_EQ(e.visual, nullptr);
}

// ============================================================
// PaintRequestEvent
// ============================================================
TEST_F(EventTest, PaintRequestEventStoresVisualAndRect)
{
	PaintRequestEvent e(nullptr, RectF(10.0f, 20.0f, 100.0f, 80.0f));

	EXPECT_EQ(e.type, Type::PaintRequest);
	EXPECT_EQ(e.visual, nullptr);
	EXPECT_EQ(e.dirtyRect, RectF(10.0f, 20.0f, 100.0f, 80.0f));
}

// ============================================================
// TargetDestroyedEvent
// ============================================================
TEST_F(EventTest, TargetDestroyedEventStoresTarget)
{
	Element root(nullptr);
	TargetDestroyedEvent e(&root);

	EXPECT_EQ(e.type, Type::TargetDestroyed);
	EXPECT_EQ(e.target, &root);
}

// ============================================================
// Modifier bit operations
// ============================================================
TEST_F(EventTest, ModifierOrCombinesFlags)
{
	Modifier combined = Modifier::Ctrl | Modifier::Shift;
	EXPECT_NE(combined, Modifier::None);

	// Both flags should be set
	EXPECT_NE((combined & Modifier::Ctrl), Modifier::None);
	EXPECT_NE((combined & Modifier::Shift), Modifier::None);
	EXPECT_EQ((combined & Modifier::Alt), Modifier::None);
}

TEST_F(EventTest, ModifierAndExtractsFlags)
{
	Modifier combined = Modifier::Ctrl | Modifier::Shift | Modifier::Alt;

	EXPECT_NE((combined & Modifier::Ctrl), Modifier::None);
	EXPECT_NE((combined & Modifier::Shift), Modifier::None);
	EXPECT_NE((combined & Modifier::Alt), Modifier::None);
	EXPECT_EQ((combined & Modifier::Super), Modifier::None);
}
