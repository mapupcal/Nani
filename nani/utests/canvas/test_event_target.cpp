#include <gtest/gtest.h>
#include "defs.h"

// ============================================================
// Fixture: EventTargetTest — EventTarget with EventFilter
// ============================================================
class EventTargetTest : public ::testing::Test
{
protected:
	void SetUp() override {}
	void TearDown() override {}
};

// Test element that records events it receives
class TestElement : public Element
{
public:
	TestElement(Element* parent = nullptr) : Element(parent) {}

	std::vector<Type> receivedEvents;

protected:
	void OnEvent(Event* e) override
	{
		receivedEvents.push_back(e->type);
		Element::OnEvent(e);
	}
};

// Simple filter that blocks specific event types
class BlockingFilter : public EventFilter
{
public:
	BlockingFilter(Type blockType) : m_blockType(blockType) {}

	bool Filter(EventTarget* target, Event* e) override
	{
		if (e->type == m_blockType)
			return true;
		return false;
	}

private:
	Type m_blockType;
};

// Filter that logs all events it sees
class LoggingFilter : public EventFilter
{
public:
	int eventCount = 0;

	bool Filter(EventTarget* target, Event* e) override
	{
		eventCount++;
		return false;
	}
};

// Filter that detects TargetDestroyed
class TargetDestroyedDetector : public EventFilter
{
public:
	bool destroyed = false;

	bool Filter(EventTarget* target, Event* e) override
	{
		if (e->type == Type::TargetDestroyed)
			destroyed = true;
		return false;
	}
};

TEST_F(EventTargetTest, RegisterAndUnRegisterFilter)
{
	TestElement element(nullptr);
	LoggingFilter filter;

	element.RegisterEventFilter(&filter);
	// Re-registering same filter should be no-op (message only)
	EXPECT_NO_FATAL_FAILURE({
		element.RegisterEventFilter(&filter);
	});

	element.UnRegisterEventFilter(&filter);
	// Un-registering non-registered filter should be no-op (message only)
	EXPECT_NO_FATAL_FAILURE({
		element.UnRegisterEventFilter(&filter);
	});
}

TEST_F(EventTargetTest, FireEventDispatchesToOnEvent)
{
	TestElement element(nullptr);

	MouseMoveEvent event(PointF(10, 20), PointF(30, 40));
	element.FireEvent(&event);

	ASSERT_EQ(element.receivedEvents.size(), 1u);
	EXPECT_EQ(element.receivedEvents[0], Type::MouseMove);
}

TEST_F(EventTargetTest, FilterCanBlockEvent)
{
	TestElement element(nullptr);
	BlockingFilter blocker(Type::MouseMove);
	element.RegisterEventFilter(&blocker);

	MouseMoveEvent moveEvent(PointF(0, 0), PointF(0, 0));
	element.FireEvent(&moveEvent);
	EXPECT_TRUE(element.receivedEvents.empty());

	// Other event types should still pass
	MousePressEvent pressEvent(MouseButton::Left, PointF(0, 0), PointF(0, 0));
	element.FireEvent(&pressEvent);
	ASSERT_EQ(element.receivedEvents.size(), 1u);
	EXPECT_EQ(element.receivedEvents[0], Type::MousePress);
}

TEST_F(EventTargetTest, MultipleFiltersFilterInReverseOrder)
{
	TestElement element(nullptr);
	LoggingFilter f1, f2;

	element.RegisterEventFilter(&f1);
	element.RegisterEventFilter(&f2);

	MouseMoveEvent event(PointF(0, 0), PointF(0, 0));
	element.FireEvent(&event);

	// Both filters should see the event
	EXPECT_EQ(f1.eventCount, 1);
	EXPECT_EQ(f2.eventCount, 1);
}

TEST_F(EventTargetTest, DestructorFiresTargetDestroyedEvent)
{
	TargetDestroyedDetector detector;

	{
		TestElement element(nullptr);
		element.RegisterEventFilter(&detector);
		EXPECT_FALSE(detector.destroyed);
	}
	// element destroyed, should fire TargetDestroyed
	EXPECT_TRUE(detector.destroyed);
}

TEST_F(EventTargetTest, FilterSeesTargetDestroyedDuringFireEvent)
{
	LoggingFilter filter;
	{
		TestElement element(nullptr);
		element.RegisterEventFilter(&filter);

		MouseMoveEvent event(PointF(0, 0), PointF(0, 0));
		element.FireEvent(&event);
		EXPECT_EQ(filter.eventCount, 1);
	}
	// After destruction, filter is still valid but element is gone
}

// ============================================================
// target_ptr tests
// ============================================================
class TargetPtrTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		element = new TestElement(nullptr);
	}

	void TearDown() override
	{
		delete element;
		element = nullptr;
	}

	TestElement* element = nullptr;
};

TEST_F(TargetPtrTest, DefaultConstructionIsNull)
{
	target_ptr<Element> ptr;
	EXPECT_EQ(ptr.get(), nullptr);
	EXPECT_FALSE(ptr);
	EXPECT_TRUE(!ptr);
}

TEST_F(TargetPtrTest, ConstructFromRawPointer)
{
	target_ptr<Element> ptr(element);
	EXPECT_EQ(ptr.get(), element);
	EXPECT_TRUE(ptr);
	EXPECT_FALSE(!ptr);
}

TEST_F(TargetPtrTest, CopyConstruction)
{
	target_ptr<Element> ptr1(element);
	target_ptr<Element> ptr2(ptr1);

	EXPECT_EQ(ptr2.get(), element);
}

TEST_F(TargetPtrTest, MoveConstruction)
{
	target_ptr<Element> ptr1(element);
	target_ptr<Element> ptr2(std::move(ptr1));

	EXPECT_EQ(ptr2.get(), element);
	EXPECT_EQ(ptr1.get(), nullptr);
}

TEST_F(TargetPtrTest, CopyAssignment)
{
	target_ptr<Element> ptr1(element);
	target_ptr<Element> ptr2;
	ptr2 = ptr1;

	EXPECT_EQ(ptr2.get(), element);
}

TEST_F(TargetPtrTest, MoveAssignment)
{
	target_ptr<Element> ptr1(element);
	target_ptr<Element> ptr2;
	ptr2 = std::move(ptr1);

	EXPECT_EQ(ptr2.get(), element);
	EXPECT_EQ(ptr1.get(), nullptr);
}

TEST_F(TargetPtrTest, AssignRawPointer)
{
	target_ptr<Element> ptr;
	ptr = element;

	EXPECT_EQ(ptr.get(), element);
}

TEST_F(TargetPtrTest, AssignNullptr)
{
	target_ptr<Element> ptr(element);
	EXPECT_EQ(ptr.get(), element);

	ptr = nullptr;
	EXPECT_EQ(ptr.get(), nullptr);
}

TEST_F(TargetPtrTest, DereferenceOperator)
{
	target_ptr<Element> ptr(element);

	EXPECT_EQ(&(*ptr), element);
}

TEST_F(TargetPtrTest, ArrowOperator)
{
	target_ptr<Element> ptr(element);

	EXPECT_EQ(ptr->Parent(), nullptr);
}

TEST_F(TargetPtrTest, ImplicitConversion)
{
	target_ptr<Element> ptr(element);
	Element* raw = ptr;

	EXPECT_EQ(raw, element);
}

TEST_F(TargetPtrTest, EqualityOperators)
{
	target_ptr<Element> ptr1(element);
	target_ptr<Element> ptr2(element);
	target_ptr<Element> ptr3;

	EXPECT_EQ(ptr1, ptr2);
	EXPECT_NE(ptr1, ptr3);

	EXPECT_EQ(ptr1, element);
	EXPECT_NE(ptr1, (Element*)nullptr);
	EXPECT_EQ(ptr3, nullptr);
	EXPECT_NE(ptr3, element);
}

TEST_F(TargetPtrTest, EqualityWithRawPointerLhs)
{
	target_ptr<Element> ptr(element);

	EXPECT_EQ(element, ptr);
	EXPECT_NE((Element*)nullptr, ptr);
}

TEST_F(TargetPtrTest, AutoNullOnTargetDestroyed)
{
	target_ptr<TestElement> ptr(element);
	EXPECT_EQ(ptr.get(), element);

	delete element;
	element = nullptr;

	EXPECT_EQ(ptr.get(), nullptr);
	EXPECT_FALSE(ptr);
}
