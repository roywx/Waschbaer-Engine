
# Scripting API Reference

Reference for the Lua scripting surface exposed by the engine. Components are Lua tables with lifecycle methods; the engine calls these methods and provides the namespaces below as globals.

In all examples, `self` refers to the component instance and `self.actor` refers to the owning actor.

There may be inaccuraries present so if you spot any feel free to reach out.

---

## Table of Contents

**Lifecycle**
- [Lifecycle Hooks](#lifecycle-hooks)

**Classes**
- [Actor](#actor-class)
- [Vector2](#vector2)
- [Rigidbody](#rigidbody)
- [ParticleSystem](#particlesystem)
- [Collision](#collision)
- [HitResult](#hitresult)

**Namespaces**
- [Debug](#debug)
- [Application](#application)
- [Input](#input)
- [Actor](#actor-namespace)
- [Scene](#scene)
- [Text](#text)
- [Audio](#audio)
- [Image](#image)
- [Camera](#camera)
- [Physics](#physics)
- [Event](#event)

---
## Defining a Component

A component is a Lua global assigned a table literal whose fields are data and lifecycle methods. The file's name must match the global's name — `MyComponent.lua` defines `MyComponent`. 
```lua

MyComponent = {
	-- Default field values.
	speed = 5,

	OnStart = function(self)
	self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self)
	self.transform.x = self.transform.x + self.speed
end
}
```
## Lifecycle Hooks

A component may implement any of the following methods. The engine calls them automatically; you never invoke them yourself.

| Hook | When it fires |
|---|---|
| `OnStart(self)` | Once, the first frame the component is active on an actor. |
| `OnUpdate(self)` | Every frame, in key order across components on an actor. |
| `OnLateUpdate(self)` | Every frame, after all `OnUpdate` calls have run. |
| `OnDestroy(self)` | Once, when the component or its actor is destroyed. |
| `OnCollisionEnter(self, collision)` | Two colliders begin contact. See [Collision](#collision). |
| `OnCollisionExit(self, collision)` | Two colliders end contact. |
| `OnTriggerEnter(self, collision)` | Two triggers begin overlap. |
| `OnTriggerExit(self, collision)` | Two triggers end overlap. |

---

# Classes

## Actor Class

Represents an entity in the game world. Obtained via `Actor.Find`, `Actor.FindAll`, `Actor.Instantiate`, or `self.actor` inside a component.

### `GetName()`

Returns the name of the actor.

```lua
local name = self.actor:GetName()
```

### `GetID()`

Returns the integer ID of the actor. IDs are unique and monotonically increasing.

```lua
local id = self.actor:GetID()
```

### `GetComponent(type_name)`

Returns a reference to the first component of type `type_name` on the actor, or `nil` if no such component exists.

```lua
local rb = self.actor:GetComponent("Rigidbody")
```

### `GetComponents(type_name)`

Returns a 1-indexed table of all components of type `type_name` on the actor. Returns an empty table if no components match.

```lua
local renderers = self.actor:GetComponents("SpriteRenderer")
for i, r in ipairs(renderers) do
    r.sprite = "new_sprite"
end
```

### `GetComponentByKey(key)`

Returns the component with the given key, or `nil` if no such component exists. Component keys are assigned in the scene/template JSON (e.g., `"1"`, `"2"`) or auto-generated at runtime (e.g., `"r0"`, `"r1"`).

```lua
local first = self.actor:GetComponentByKey("1")
```

### `AddComponent(type_name)`

Adds a component of type `type_name` to the actor and returns a reference to it. The new component's `OnStart` runs on the next frame. The new component's key is `"rN"`, where `N` is the total number of runtime additions since program start.

```lua
local sr = self.actor:AddComponent("SpriteRenderer")
sr.sprite = "new_sprite"
```

### `RemoveComponent(component_ref)`

Removes a component from the actor. The component is marked disabled immediately; its `OnDestroy` runs at the end of the current frame.

```lua
local sr = self.actor:GetComponent("SpriteRenderer")
self.actor:RemoveComponent(sr)
```

---

## Vector2

2D mathematical vector. Supports `+`, `-`, and scalar `*`.

### Members

- `x` — number, x component.
- `y` — number, y component.

### Constructor

```lua
local v = Vector2(2, 5)
```

### `Normalize()`

Converts the vector into a unit vector in place. Returns the original length.

```lua
local v = Vector2(2, 5)
v:Normalize()
```

### `Length()`

Returns the length of the vector.

```lua
local len = v:Length()
```

### `Vector2.Distance(a, b)`

Static function. Returns the distance between two vectors, i.e. `Length(a - b)`.

```lua
local d = Vector2.Distance(Vector2(2, 5), Vector2(-1, -3))
```

### `Vector2.Dot(a, b)`

Static function. Returns the dot product of two vectors.

```lua
local dp = Vector2.Dot(Vector2(2, 5), Vector2(-1, -3))
```

---

## Rigidbody

Built-in C++ component that wraps a Box2D body. Added to actors via `"type": "Rigidbody"` in JSON, or `actor:AddComponent("Rigidbody")` at runtime.

### Configuration properties

Set these **before** `OnStart` — typically in the actor template or scene JSON. `OnStart` consumes them to construct the Box2D body; changing most of them after `OnStart` has no effect.

| Property | Type | Description |
|---|---|---|
| `x`, `y` | number | Initial world position. |
| `body_type` | string | `"dynamic"`, `"static"`, or `"kinematic"`. |
| `precise` | bool | Enables Box2D bullet mode for fast-moving bodies. |
| `gravity_scale` | number | World gravity is `(0, 9.8)`. Positive Y is **down**. |
| `density` | number | Applied to all fixtures. |
| `angular_friction` | number | Box2D angular damping. |
| `rotation` | number | Initial rotation, **degrees clockwise**. |
| `has_collider` | bool | If `false` and `has_trigger` is also `false`, a hidden phantom sensor is attached so the body still integrates. |
| `has_trigger` | bool | Triggers are separate sensor fixtures. |
| `collider_type` | string | `"box"` or `"circle"`. |
| `width`, `height` | number | Full width/height for box colliders. |
| `radius` | number | Radius for circle colliders. |
| `friction` | number | Collider friction. |
| `bounciness` | number | Collider restitution. |
| `trigger_type` | string | `"box"` or `"circle"`. |
| `trigger_width`, `trigger_height` | number | Full width/height for box triggers. |
| `trigger_radius` | number | Radius for circle triggers. |

**Collision filtering.** Colliders only interact with colliders; triggers only interact with triggers. A collider will **not** fire `OnTriggerEnter` on an overlapping trigger fixture, and vice versa.

### Runtime properties

| Property | Type | Description |
|---|---|---|
| `actor` | Actor | Owning actor. |
| `enabled` | bool | If `false`, lifecycle hooks are skipped. |
| `key` | string | The component's key on its actor. |

### Forces and velocity

- **`AddForce(vec)`** — applies force to the body's center. `vec` is a `Vector2`. Before `OnStart`, forces accumulate in a pending buffer.
- **`SetVelocity(vec)`** — sets linear velocity.
- **`SetAngularVelocity(deg)`** — sets angular velocity in degrees per step, clockwise.
- **`SetGravityScale(scale)`** — overrides the body's gravity scale.

### Transform

- **`SetPosition(vec)`** — teleports the body. Before `OnStart`, updates the initial `x`/`y`.
- **`SetRotation(deg)`** — sets rotation in degrees clockwise.
- **`SetUpDirection(vec)`** — rotates the body so its local "up" points along `vec`. `vec` is normalized internally.
- **`SetRightDirection(vec)`** — rotates the body so its local "right" points along `vec`.

### Queries

- **`GetPosition()`** → `Vector2`
- **`GetRotation()`** → number (degrees clockwise)
- **`GetVelocity()`** → `Vector2`
- **`GetAngularVelocity()`** → number (degrees per step)
- **`GetGravityScale()`** → number
- **`GetUpDirection()`** → `Vector2` (unit vector)
- **`GetRightDirection()`** → `Vector2` (unit vector)

> `GetVelocity`, `GetAngularVelocity`, `GetGravityScale`, `GetUpDirection`, and `GetRightDirection` dereference the Box2D body directly. Calling them before `OnStart` has created the body will crash.

```lua
MoveRight = {
	OnUpdate()
	    local rb = self.actor:GetComponent("Rigidbody")
	    rb:AddForce(Vector2(5, 0))
	end
}
```

---

## ParticleSystem

Built-in C++ component that spawns and simulates 2D particles. Particles draw with the texture named by `image` at the given `sorting_order`. If `image` is the empty string, an 8×8 white fallback texture is used.

### Emission and lifetime

| Property | Type | Description |
|---|---|---|
| `x`, `y` | number | World-space center of the emitter. |
| `frames_between_bursts` | int | Cadence of bursts. Clamped to 1 if ≤ 0. |
| `burst_quantity` | int | Particles spawned per burst. Clamped to 1 if ≤ 0. |
| `duration_frames` | int | Particle lifetime in frames. Clamped to 1 if ≤ 0. |
| `emit_radius_min`, `emit_radius_max` | number | Spawn ring inner/outer radius around `(x, y)`. |
| `emit_angle_min`, `emit_angle_max` | number | Spawn angle range in **degrees**. |
| `image` | string | Image name; empty string uses the fallback. |
| `sorting_order` | int | Render sort order. |

### Initial state (randomized per particle)

| Property | Type | Description |
|---|---|---|
| `start_scale_min`, `start_scale_max` | number | Uniform starting scale. |
| `rotation_min`, `rotation_max` | number | Starting rotation in degrees. |
| `start_speed_min`, `start_speed_max` | number | Speed along the emission direction. |
| `rotation_speed_min`, `rotation_speed_max` | number | Angular velocity in degrees per frame. |
| `start_color_r/g/b/a` | number | 0–255 each. |

### Per-frame forces

| Property | Type | Description |
|---|---|---|
| `gravity_scale_x`, `gravity_scale_y` | number | Added to velocity every frame. Effectively an acceleration vector, despite the name. |
| `drag_factor` | number | Velocity is multiplied by this each frame. `1.0` = no drag. |
| `angular_drag_factor` | number | Same as `drag_factor` but for angular velocity. |

### End state (interpolated over lifetime)

Particles linearly interpolate from initial values to these end values over `duration_frames`.

| Property | Type | Description |
|---|---|---|
| `end_scale` | number | Target scale. If unset, particles keep their initial scale. |
| `end_color_r/g/b/a` | number | Target color channels. Any unset channel falls back to the matching `start_color_*`. |

### Methods

- **`OnStart()`** — builds random-number distributions from the `min`/`max` properties and resolves end colors. Called by the engine.
- **`OnUpdate()`** — bursts if enabled, simulates active particles in parallel, then submits draw calls. Called by the engine.
- **`Play()`** — enables emission.
- **`Stop()`** — disables emission. Existing particles live out their remaining lifetime.
- **`Burst()`** — emits `burst_quantity` particles immediately, regardless of emission state or burst cadence.

```lua
Explosion =  { 
	OnTriggerEnter =  function(self, collision) 
		local ps = self.actor:GetComponent("ParticleSystem") 
		ps:Burst() 
	end 
}
```

---

## Collision

Passed to `OnCollisionEnter`, `OnCollisionExit`, `OnTriggerEnter`, and `OnTriggerExit`.

| Property | Type | Description |
|---|---|---|
| `other` | Actor | The other actor in the contact. |
| `point` | Vector2 | World-space contact point. **Sentinel value for trigger contacts and `OnCollisionExit`** — not meaningful there. |
| `relative_velocity` | Vector2 | Velocity of this body minus velocity of the other, at the time of contact. |
| `normal` | Vector2 | Contact normal. **Sentinel value for trigger contacts and `OnCollisionExit`.** |

The engine uses a sentinel `Vector2` (see `SENTINEL_VEC` in `Actor.h`) for `point` and `normal` when those values aren't available. Check against it before using them.

---

## HitResult

Returned by `Physics.Raycast` and contained in the table returned by `Physics.RaycastAll`.

| Property | Type | Description |
|---|---|---|
| `actor` | Actor | The actor owning the fixture that was hit. |
| `point` | Vector2 | World-space intersection point. |
| `normal` | Vector2 | Surface normal at the hit point. |
| `is_trigger` | bool | `true` if the hit fixture is a sensor (trigger), `false` if a solid collider. |

---

# Namespaces

## Debug

### `Debug.Log(message)`

Prints a message to stdout, followed by a newline.

```lua
Debug.Log("Player position: " .. rb:GetPosition().x)
```

---

## Application

### `Application.Quit()`

Exits the application immediately with code 0.

```lua
Application.Quit()
```

### `Application.Sleep(milliseconds)`

Blocks the current thread for the specified number of milliseconds. Use sparingly — this freezes the entire game loop.

```lua
Application.Sleep(1000)
```

### `Application.GetFrame()`

Returns the current frame number as an integer.

```lua
local frame = Application.GetFrame()
```

### `Application.OpenURL(url)`

Opens the given URL in the user's default browser.

```lua
Application.OpenURL("http://www.example.com")
```

---

## Input

Named keys accepted by the keyboard functions include: `"up"`, `"down"`, `"left"`, `"right"`, `"escape"`, `"space"`, `"return"` / `"enter"`, `"backspace"`, `"tab"`, `"lshift"`, `"rshift"`, `"lctrl"`, `"rctrl"`, `"lalt"`, `"ralt"`, letter keys `"a"`–`"z"`, digit keys `"0"`–`"9"`, and punctuation `"/"`, `";"`, `"="`, `"-"`, `"."`, `","`, `"["`, `"]"`, `"\\"`, `"'"`.

### `Input.GetKey(keycode)`

Returns `true` if the key is currently held down.

```lua
if Input.GetKey("space") then ... end
```

### `Input.GetKeyDown(keycode)`

Returns `true` only on the frame the key became pressed.

```lua
if Input.GetKeyDown("a") then ... end
```

### `Input.GetKeyUp(keycode)`

Returns `true` only on the frame the key was released.

```lua
if Input.GetKeyUp("up") then ... end
```

### `Input.GetMousePosition()`

Returns the current mouse position as a `vec2` with `x` and `y` fields (window pixel coordinates).

```lua
local pos = Input.GetMousePosition()
Debug.Log("X: " .. pos.x .. ", Y: " .. pos.y)
```

### `Input.GetMouseButton(button_num)`

Returns `true` if the mouse button is currently held. `1` = left, `2` = middle, `3` = right.

```lua
if Input.GetMouseButton(1) then ... end
```

### `Input.GetMouseButtonDown(button_num)`

Returns `true` only on the frame the button was pressed.

```lua
if Input.GetMouseButtonDown(3) then ... end
```

### `Input.GetMouseButtonUp(button_num)`

Returns `true` only on the frame the button was released.

```lua
if Input.GetMouseButtonUp(2) then ... end
```

### `Input.GetMouseScrollDelta()`

Returns the scroll wheel delta for the current frame as a float. `0` if there was no scroll.

```lua
local delta = Input.GetMouseScrollDelta()
```

### `Input.HideCursor()` / `Input.ShowCursor()`

Hides or shows the OS cursor.

```lua
Input.HideCursor()
```

---

## Actor (namespace)

Not to be confused with the [Actor class](#actor-class) — this namespace contains global actor-management functions.

### `Actor.Find(name)`

Returns the first actor with the given name, or `nil` if none exists. Searches both loaded actors and actors instantiated this frame.

```lua
local player = Actor.Find("Player")
```

### `Actor.FindAll(name)`

Returns a 1-indexed table of every actor with the given name. Empty table if none exist.

```lua
local enemies = Actor.FindAll("Enemy")
for i, e in ipairs(enemies) do ... end
```

### `Actor.Instantiate(actor_template_name)`

Creates a new actor from the named template and returns a reference to it. The actor's components begin running lifecycle functions on the next frame, but the actor is discoverable via `Actor.Find` / `Actor.FindAll` immediately.

```lua
local enemy = Actor.Instantiate("Enemy")
```

### `Actor.Destroy(actor)`

Destroys an actor and all its components. No further lifecycle functions run after this call. The actual destruction happens at the end of the current frame, after all `OnLateUpdate` calls.

```lua
Actor.Destroy(self.actor)
```

---

## Scene

### `Scene.Load(scene_name)`

Loads a new scene at the beginning of the next frame. If called multiple times in one frame, only the last call takes effect.

```lua
Scene.Load("level_2")
```

### `Scene.GetCurrent()`

Returns the name of the current scene (without the `.scene` extension).

```lua
local name = Scene.GetCurrent()
```

### `Scene.DontDestroy(actor)`

Marks an actor to persist across scene loads. Useful for global manager actors.

```lua
Scene.DontDestroy(self.actor)
```

---

## Text

### `Text.Draw(str_content, x, y, font_name, font_size, r, g, b, a)`

Draws text at the given screen coordinates. Must be called every frame to remain visible.

**Parameters**
- `str_content` — string. The text to draw.
- `x`, `y` — numbers. Screen coordinates (top-left of the text).
- `font_name` — string. Name of the font file under `resources/fonts/` (without `.ttf`).
- `font_size` — integer. Point size.
- `r`, `g`, `b`, `a` — integers 0–255. Text color.

```lua
Text.Draw("Hello, World!", 100, 50, "Arial", 24, 255, 255, 255, 255)
```

---

## Audio

### `Audio.Play(channel, clip_name, does_loop)`

Plays an audio clip on a channel.

**Parameters**
- `channel` — integer. The channel to play on.
- `clip_name` — string. Name of the clip under `resources/audio/` (without extension; `.wav` and `.ogg` are both supported).
- `does_loop` — bool. `true` to loop indefinitely, `false` to play once.

```lua
Audio.Play(1, "background_music", true)
```

### `Audio.Halt(channel)`

Stops playback on the given channel.

```lua
Audio.Halt(1)
```

### `Audio.SetVolume(channel, volume)`

Sets the volume of an audio channel. `volume` is an integer from 0 (silent) to 128 (maximum).

```lua
Audio.SetVolume(1, 64)
```

---

## Image

### `Image.DrawUI(image_name, x, y)`

Draws an image to the UI layer in screen coordinates, unaffected by the camera. White tint, full opacity, sorting order 0. Must be called every frame.

**Parameters**
- `image_name` — string. Name of the image under `resources/images/` (without `.png`).
- `x`, `y` — numbers. Screen coordinates.

```lua
Image.DrawUI("logo", 100, 200)
```

### `Image.DrawUIEx(image_name, x, y, r, g, b, a, sorting_order)`

Extended UI draw with tint, alpha, and sort order. Color parameters are floats 0–255 but downcast to integers internally.

```lua
Image.DrawUIEx("hero_icon", 400, 300, 255, 0, 0, 128, 1)
```

### `Image.Draw(image_name, x, y)`

Draws an image in scene coordinates (affected by camera position and zoom). The image's pivot is centered. Must be called every frame.

```lua
Image.Draw("background", 0, 0)
```

### `Image.DrawEx(image_name, x, y, rotation_degrees, scale_x, scale_y, pivot_x, pivot_y, r, g, b, a, sorting_order)`

Extended scene draw with rotation, scale, pivot, tint, alpha, and sort order.

**Parameters**
- `image_name` — string.
- `x`, `y` — numbers. Scene coordinates.
- `rotation_degrees` — number. Clockwise rotation angle.
- `scale_x`, `scale_y` — numbers. `1.0` is natural size. Negative values flip.
- `pivot_x`, `pivot_y` — numbers 0–1. Pivot as a fraction of image size.
- `r`, `g`, `b`, `a` — numbers 0–255.
- `sorting_order` — integer.

```lua
Image.DrawEx("enemy_sprite", 500, 500, 45, 1.0, 1.0, 0.5, 0.5, 255, 255, 255, 255, 2)
```

### `Image.DrawPixel(x, y, r, g, b, a)`

Draws a single pixel at the given screen coordinates.

```lua
Image.DrawPixel(500, 500, 255, 255, 255, 255)
```

---

## Camera

### `Camera.SetPosition(x, y)`

Sets the camera's scene position.

```lua
Camera.SetPosition(100.0, 200.0)
```

### `Camera.GetPositionX()` / `Camera.GetPositionY()`

Return the camera's current x / y position as a float.

```lua
local x = Camera.GetPositionX()
local y = Camera.GetPositionY()
```

### `Camera.SetZoom(zoom_factor)`

Sets the camera's zoom. Affects `Image.Draw` and `Image.DrawEx` but **not** `Image.DrawUI`/`DrawUIEx`.

```lua
Camera.SetZoom(1.5)
```

### `Camera.GetZoom()`

Returns the current zoom factor.

```lua
local zoom = Camera.GetZoom()
```

---

## Physics

Global namespace for raycasting. Raycasts respect both colliders and triggers — check `HitResult.is_trigger` to tell them apart.

### `Physics.Raycast(pos, dir, dist)`

Fires a single ray and returns the **closest** `HitResult`, or `nil` if nothing was hit.

**Parameters**
- `pos` — `Vector2`. World-space origin.
- `dir` — `Vector2`. Direction (not required to be normalized; endpoint is `pos + dist * dir`).
- `dist` — number. Distance along `dir`.

```lua
local hit = Physics.Raycast(Vector2(0, 0), Vector2(1, 0), 10.0)
if hit ~= nil then
    Debug.Log("Hit: " .. hit.actor:GetName())
end
```

### `Physics.RaycastAll(pos, dir, dist)`

Returns every fixture the ray passes through, sorted nearest-first. Empty table if no hits.

```lua
local hits = Physics.RaycastAll(Vector2(0, 0), Vector2(0, 1), 20.0)
for i, h in ipairs(hits) do
    Debug.Log(i .. ": " .. h.actor:GetName())
end
```

---

## Event

Global pub/sub bus. Subscriptions and unsubscriptions are **deferred to the end of the frame**, so it is safe to subscribe or unsubscribe from inside an event handler.

### `Event.Publish(event_type, event_object)`

Synchronously invokes every subscriber of `event_type`, passing `event_object` through unchanged.

```lua
Event.Publish("PlayerDied", { score = 42 })
```

### `Event.Subscribe(event_type, component, function)`

Registers a subscriber. Takes effect next frame.

**Parameters**
- `event_type` — string. Channel name.
- `component` — the table passed as `self` to the handler. Usually `self` from the calling component.
- `function` — the handler. Invoked as `function(component, event_object)`.

```lua
MyComponent = {
    OnStart = function(self)
        Event.Subscribe("PlayerDied", self, self.HandlePlayerDied)
    end,

    HandlePlayerDied = function(self, event)
        Debug.Log("Player died with score " .. event.score)
    end
}
```

### `Event.Unsubscribe(event_type, component, function)`

Removes a subscription. Matches on both the component reference and the function reference — must match the triple used in `Subscribe`. Takes effect next frame.

```lua
Event.Unsubscribe("PlayerDied", self, self.HandlePlayerDied)
```
