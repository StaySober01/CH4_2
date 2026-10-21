"""Run after rebuilding M04 C++ and restarting Unreal Editor.
Uses a native editor bridge: WidgetBlueprint.WidgetTree is not exposed to Python.
Empty assets left by an earlier failed run are filled; nonempty layouts are preserved.
"""
import unreal

DESTINATION = "/Game/UI"
CONTROLLER = "/Game/VehicleTemplate/Blueprints/BP_VehicleAdvPlayerController"


def require(value, message):
    if not value:
        raise RuntimeError(message)
    return value


bridge = getattr(unreal, "CH4_2WidgetAuthoringLibrary", None)
require(bridge, "Close the editor, rebuild Development Editor / Win64, then restart. The new C++ widget authoring bridge is not loaded.")

specs = [
    ("WBP_PlayerStats", "/Script/CH4_2.CH4_2PlayerStatsWidget", False),
    ("WBP_Chat", "/Script/CH4_2.CH4_2ChatWidget", True),
]
parents = {
    name: require(unreal.load_class(None, path), f"Build C++ and restart the editor first: {path}")
    for name, path, _ in specs
}
controller = require(unreal.load_asset(CONTROLLER), "Template player controller BP is missing")
require(bridge.compile_interface_blueprint(controller), "Resolve controller Blueprint compiler errors first")
controller_cdo = unreal.get_default_object(controller.generated_class())
controller_cdo.get_editor_property("player_stats_widget_class")
controller_cdo.get_editor_property("chat_widget_class")

# Check parent classes before making any new assets. Do not reparent user assets.
for name, _, _ in specs:
    path = f"{DESTINATION}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        existing = require(unreal.load_asset(path), f"Cannot load {path}")
        require(existing.get_editor_property("parent_class") == parents[name],
                f"{path}: unexpected parent; existing asset was preserved")

assets = {}
tools = unreal.AssetToolsHelpers.get_asset_tools()
for name, _, is_chat in specs:
    path = f"{DESTINATION}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        bp = require(unreal.load_asset(path), f"Cannot load {path}")
    else:
        factory = unreal.WidgetBlueprintFactory()
        factory.set_editor_property("parent_class", parents[name])
        bp = require(tools.create_asset(name, DESTINATION, unreal.WidgetBlueprint, factory),
                     f"Cannot create {path}")
    require(bridge.prepare_widget_blueprint(bp, is_chat),
            f"Cannot prepare {path}. Read LogCH4_2 / Blueprint Compiler Results; nonempty layouts are preserved.")
    require(unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False), f"Cannot save {path}")
    assets[name] = bp.generated_class()

controller.modify()
controller_cdo = unreal.get_default_object(controller.generated_class())
controller_cdo.modify()
controller_cdo.set_editor_property("player_stats_widget_class", assets["WBP_PlayerStats"])
controller_cdo.set_editor_property("chat_widget_class", assets["WBP_Chat"])
require(bridge.compile_interface_blueprint(controller), "Resolve player controller Blueprint compiler messages")
require(unreal.EditorAssetLibrary.save_loaded_asset(controller, only_if_is_dirty=False), "Cannot save controller BP")
unreal.log("M04 WBP assets saved and assigned. Open Designer to customize; gameplay was not tested.")
