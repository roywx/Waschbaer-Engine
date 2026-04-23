# Demo Game for `waschbaer_engine`

A small playable game included with the engine as a working example. It exercises most of the engine's core systems — scenes, actor templates, Lua components, input, audio, text, and UI — so readers can see how a real game fits together before writing their own.

## Running the demo

1. Copy the `demo_game/` folder from this directory up next to the engine executable and rename it to `resources`:
```sh
cp -r demo/demo_game .
mv demo_game resources
```

(Or in Finder / File Explorer: drag `demo/demo_game/` into the project root, then rename it to `resources`.)

2. Run the engine from the root:
```sh
./waschbaer_engine
```