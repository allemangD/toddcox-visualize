I have some "composite" objects that I'm not sure how best to map into an ECS. Each composite object has multiple "parts" with various properties. But the parts of a given composite object are intrinsically linked: they transform together and use the same buffers during rendering.

Note that I'm still pretty early in adopting `entt` for this project, so I'm not particularly tied to any of the architecture here. The only constraint is that, for optimization reasons, I need each composite object to get _one_ buffer on GPU, and each part owns some interval within its parent's buffer. This nested scheme seems the most straightforward way to do that.

---

My first attempt has only one entity for each composite object, with a `Parts` component that holds an `std::vector<Part>`. 

Rendering code looks something like this. In reality I save the draw commands in a separate component and re-use those on each frame.

```c++
auto view = registry.view<Parts>();

for (auto [entity, parts]: view.each()) {
  std::vector<Command> commands;

  for (auto part: parts.parts) {
    // assemble draw command
    commands.emplace_back(...);
  }

  // bind buffers for entity
  // issue draw command
}
```

However it seems appropriate to give each part its own entity. My attempt for this was to switch to `std::vector<entt::entity>` where each entity has a `Part` as a component.

Rendering code then looks like:

```c++
auto view = registry.view<Parts>();

for (auto [entity, parts]: view.each()) {
  std::vector<Command> commands;

  for (auto part_entity: parts.parts) {
    auto &part = registry.get<Part>(part_entity);
    // assemble draw command
    commands.emplace_back(...);
  }

  // bind buffers for entity
  // issue draw command with shared buffer
}
```

---

In my last attempt, I invert this so each `Part` holds only parent entity. The `Parts` component is removed. I can then use a `view<Part>` directly, but it's trickier to organize the draw calls since they must be grouped by the parent entity itself.

```c++
auto view = registry.view<Part>();

for (auto [entity, part]: view.each()) {
  // assemble draw command
  auto &commands = registry.get<Commands>(part.parent);
  commands.emplace_back(...);
}

for (auto [entity, commands]: view
for (auto [entity, commands]: commands) {
   // bind buffers for entity
   auto &commands = registry.get<Commands>(entity);
   // issue draw commands
}
```

---

Is there any more sensible approach to this kind of many-to-one entity composition? Or some way to declare _multiple_ `Part` components on the same entity? It seems like I almost need some group-by kind of operation, but I don't see such on the registry/view docs.