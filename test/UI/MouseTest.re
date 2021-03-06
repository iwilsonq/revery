open Rejest;

open Revery_UI;
open UiEvents;

let createNodeWithStyle = style => {
  let node = (new node)();
  node#setStyle(style);
  Layout.layout(node, 1.0);
  node#recalculate();
  node;
};

test("Mouse", () => {
  test("dispatch", () => {
    test("triggers onMouseDown event for node", () => {
      let cursor = Mouse.Cursor.make();

      let count = ref(0);
      let f = _evt => count := count^ + 1;
      let node =
        createNodeWithStyle(Style.make(~width=100, ~height=100, ()));
      node#setEvents(NodeEvents.make(~onMouseDown=f, ()));

      Mouse.dispatch(
        cursor,
        InternalMouseMove({mouseX: 50., mouseY: 50.}),
        node,
      );
      Mouse.dispatch(cursor, InternalMouseDown({button: BUTTON_LEFT}), node);

      expect(count^).toBe(1);
    });
    test("does not trigger onMouseUp event for node if outside node", () => {
      let cursor = Mouse.Cursor.make();

      let count = ref(0);
      let f = _evt => count := count^ + 1;
      let node =
        createNodeWithStyle(Style.make(~width=100, ~height=100, ()));
      node#setEvents(NodeEvents.make(~onMouseDown=f, ()));

      Mouse.dispatch(
        cursor,
        InternalMouseMove({mouseX: 150., mouseY: 150.}),
        node,
      );
      Mouse.dispatch(cursor, InternalMouseUp({button: BUTTON_LEFT}), node);

      expect(count^).toBe(0);
    });
    test("does trigger onFocus for node", () => {
      let cursor = Mouse.Cursor.make();
      Mouse.Cursor.set(cursor, Revery_Math.Vec2.create(50.0, 50.0));

      let count = ref(0);
      let f = _evt => count := count^ + 1;
      let node =
        createNodeWithStyle(Style.make(~width=100, ~height=100, ()));
      node#setEvents(NodeEvents.make(~onFocus=f, ()));
      node#setTabIndex(Some(1));

      Mouse.dispatch(cursor, InternalMouseDown({button: BUTTON_LEFT}), node);

      expect(count^).toBe(1);
    });
    test(
      "does trigger onBlur for node after cursor is pressed outside the node",
      () => {
      let cursor = Mouse.Cursor.make();
      Mouse.Cursor.set(cursor, Revery_Math.Vec2.create(50.0, 50.0));

      let count = ref(0);
      let f = _evt => count := count^ + 1;
      let node =
        createNodeWithStyle(Style.make(~width=100, ~height=100, ()));
      node#setEvents(NodeEvents.make(~onBlur=f, ()));
      node#setTabIndex(Some(1));
      Mouse.dispatch(cursor, InternalMouseDown({button: BUTTON_LEFT}), node);
      Mouse.Cursor.set(cursor, Revery_Math.Vec2.create(200.0, 200.0));
      Mouse.dispatch(cursor, InternalMouseDown({button: BUTTON_LEFT}), node);
      expect(count^).toBe(1);
    });
  });

  test("bubbleEvent", () => {
    test(
      "test that state is updated per event when stop propagation is called",
      () => {
      let evt = BubbledEvent.make(MouseMove({mouseX: 50., mouseY: 50.}));
      switch (evt) {
      | Some(e) =>
        e.stopPropagation();
        switch (BubbledEvent.activeEvent^) {
        | Some(activeEvent) =>
          expect(activeEvent.shouldPropagate).toBe(false)
        | None => ()
        };
      | None => ()
      };
    });

    test(
      "test that state is updated per event when prevent default is called", () => {
      let evt = BubbledEvent.make(MouseMove({mouseX: 50., mouseY: 50.}));
      switch (evt) {
      | Some(e) =>
        e.preventDefault();
        switch (BubbledEvent.activeEvent^) {
        | Some(activeEvent) =>
          expect(activeEvent.defaultPrevented).toBe(true)
        | None => ()
        };
      | None => ()
      };
    });
  });

  test("setCapture/releaseCapture", () =>
    test("captured events override dispatching to node", () => {
      let cursor = Mouse.Cursor.make();

      let nodeCount = ref(0);
      let captureCount = ref(0);
      let nodeCounter = _evt => nodeCount := nodeCount^ + 1;
      let captureCounter = _evt => captureCount := captureCount^ + 1;

      let node =
        createNodeWithStyle(Style.make(~width=100, ~height=100, ()));

      Mouse.setCapture(~onMouseDown=captureCounter, ());

      node#setEvents(NodeEvents.make(~onMouseDown=nodeCounter, ()));

      Mouse.dispatch(
        cursor,
        InternalMouseMove({mouseX: 50., mouseY: 50.}),
        node,
      );

      Mouse.dispatch(cursor, InternalMouseDown({button: BUTTON_LEFT}), node);

      expect(nodeCount^).toBe(0);
      expect(captureCount^).toBe(1);

      Mouse.releaseCapture();
      Mouse.dispatch(cursor, InternalMouseDown({button: BUTTON_LEFT}), node);

      expect(nodeCount^).toBe(1);
      expect(captureCount^).toBe(1);
    })
  );
  test("onCursorChangedEvent gets dispatched with proper cursor", () => {
    module Cursors = Revery_Core.MouseCursors;

    let count = ref(0);
    let cursorType = ref(Cursors.arrow);
    let f = cur => {
      count := count^ + 1;
      cursorType := cur;
    };
    let node =
      createNodeWithStyle(
        Style.make(~width=100, ~cursor=Cursors.text, ~height=100, ()),
      );

    let _ = Revery_Core.Event.subscribe(Mouse.onCursorChanged, f);

    Mouse.dispatch(
      Mouse.Cursor.make(),
      InternalMouseMove({mouseX: 50., mouseY: 50.}),
      node,
    );

    expect(count^).toBe(1);
    expect(cursorType^).toBe(Cursors.text);
  });
});
