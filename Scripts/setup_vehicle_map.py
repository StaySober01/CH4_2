"""Run with UnrealEditor-Cmd -run=pythonscript -script=<this file>.

Imports are engine APIs; never edits binary assets outside Unreal.
"""
import json
from pathlib import Path
import unreal

MAP = "/Game/VehicleTemplate/Maps/VehicleBasic"
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert level.load_level(MAP), "VehicleBasic failed to load"
starts = sorted(
    (actor for actor in actors.get_all_level_actors() if isinstance(actor, unreal.PlayerStart)),
    key=lambda actor: actor.get_name(),
)
assert starts, "Template has no PlayerStart"
assert len(starts) <= 3, "Unexpected existing starts: refusing to remove user actors"
origin = starts[0].get_actor_location()
rotation = starts[0].get_actor_rotation()
while len(starts) < 3:
    location = unreal.Vector(origin.x, origin.y + 500.0 * len(starts), origin.z)
    start = actors.spawn_actor_from_class(unreal.PlayerStart, location, rotation)
    assert start, "Failed to create PlayerStart"
    start.set_actor_label("PlayerStart_Vehicle_" + str(len(starts) + 1))
    starts.append(start)
assert level.save_current_level(), "Failed to save VehicleBasic"
registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
blueprints = []
for path in ("/Game/VehicleTemplate", "/Game/Variant_OffRoad", "/Game/Variant_TimeTrial", "/Game/Vehicles", "/Game/Input"):
    for data in registry.get_assets_by_path(path, recursive=True):
        asset = data.get_asset()
        assert asset, "Failed to load " + str(data.package_name)
        if isinstance(asset, unreal.Blueprint):
            unreal.BlueprintEditorLibrary.compile_blueprint(asset)
            blueprints.append(str(data.package_name))
report = {
    "map": MAP,
    "starts": [{"name": s.get_name(), "location": str(s.get_actor_location())} for s in starts],
    "loaded_and_compile_requested_blueprints": blueprints,
    "note": "Review engine log for compile errors. Does not validate driving or network play.",
}
report_path = Path(unreal.Paths.project_saved_dir()) / "Validation" / "vehicle_map.json"
report_path.parent.mkdir(parents=True, exist_ok=True)
report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
unreal.log("VEHICLE_SETUP_PASS " + str(report_path))
