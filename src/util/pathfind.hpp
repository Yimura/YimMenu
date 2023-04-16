#pragma once

#include "gta/enums.hpp"
#include "math.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "script.hpp"

#include <cstdlib>
#include <ctime>
#include <random>


namespace big::pathfind
{

	inline bool load_path_nodes(Vector3 coords)
	{
		if (PATHFIND::ARE_NODES_LOADED_FOR_AREA(coords.x, coords.y, coords.z, coords.y))
			return true;

		PATHFIND::REQUEST_PATH_NODES_IN_AREA_THIS_FRAME(coords.x, coords.y, coords.z, coords.y);

		for (int i = 0; i < 35 && !PATHFIND::ARE_NODES_LOADED_FOR_AREA(coords.x, coords.y, coords.z, coords.y); i++)
		{
			PATHFIND::REQUEST_PATH_NODES_IN_AREA_THIS_FRAME(coords.x, coords.y, coords.z, coords.y);
			script::get_current()->yield(10ms);
		}

		return PATHFIND::ARE_NODES_LOADED_FOR_AREA(coords.x, coords.y, coords.z, coords.y);
	}

	inline bool load_navmesh_area(Vector3 coords, float radius)
	{
		if (PATHFIND::ARE_ALL_NAVMESH_REGIONS_LOADED())
			return true;

		PATHFIND::ADD_NAVMESH_REQUIRED_REGION(coords.x, coords.z, radius);

		for (int i = 0; i < 35 && !PATHFIND::ARE_ALL_NAVMESH_REGIONS_LOADED(); i++)
		{
			script::get_current()->yield(10ms);
		}

		return PATHFIND::ARE_ALL_NAVMESH_REGIONS_LOADED();
	}

	/*
	Be sure to call this after having added a required region to free up memory since the game files suggest it is rather demanding to load navmesh
	*/
	inline void remove_navmesh_required_areas()
	{
		PATHFIND::REMOVE_NAVMESH_REQUIRED_REGIONS();
	}

	inline bool find_safe_pos_ped(Vector3 coords, Vector3& outcoords, bool onGround, int flag)
	{
		if (load_path_nodes(coords))
			return PATHFIND::GET_SAFE_COORD_FOR_PED(coords.x, coords.y, coords.z, onGround, &outcoords, flag);
		else
			return false;
	}

	/*
	 zMeasureMult: how strongly should the difference in Z direction be weighted? 
	 0.0 = ignored completely, 1.0 = the same as 2d distance. Default is 3.0 since we tend to care about
	 height differences more than 2d distance.
	 zTolerance: how far apart to the Z coords have to be before zMeasureMult kicks in?
	 nth: Which coords are returned, 1 being closest, 2 being second closesst and so on
	*/
	inline bool find_closest_vehicle_node(Vector3 coords, Vector3& outcoords, float& outheading, int flag, int nth = 1, float zMeasureMult = 3.f, float zTolerance = 0.f)
	{
		int lanes;
		if (load_path_nodes(coords))
			return PATHFIND::GET_NTH_CLOSEST_VEHICLE_NODE_WITH_HEADING(coords.x, coords.y, coords.z, nth, &outcoords, &outheading, &lanes, flag, zMeasureMult, zTolerance);
		else
			return false;
	}

	inline bool find_random_vehicle_node(Vector3 center, Vector3& outcoords, float radius, bool avoid_dead_ends, bool avoid_highways, int min_lanes = 0)
	{
		int node_id;
		if (load_path_nodes(center))
			return PATHFIND::GET_RANDOM_VEHICLE_NODE(center.x, center.y, center.z, radius, 0, avoid_dead_ends, avoid_highways, &outcoords, &node_id);
		else
			return false;
	}


	inline bool find_random_location_in_vicinity(Vector3 coords, Vector3& outcoords, float& outheading, int flag, int vicinity)
	{
		srand(time(NULL));
		bool apply_to_x     = rand() % 2;
		bool apply_positive = rand() % 2;

		if (apply_to_x)
		{
			apply_positive ? outcoords.x += vicinity : outcoords.x -= vicinity;
		}
		else
		{
			apply_positive ? outcoords.y += vicinity : outcoords.y -= vicinity;
		}

		Vector3 changed_coords = outcoords;

		if (!find_closest_vehicle_node(outcoords, outcoords, outheading, flag) || math::distance_between_vectors(outcoords, coords) > vicinity || math::distance_between_vectors(outcoords, coords) < (vicinity / 2))
		{
			outcoords = coords;

			if (!pathfind::find_safe_pos_ped(changed_coords, outcoords, false, 0))
			{
				outcoords = coords;
			}
			else
			{
				LOG(INFO) << "Found safe ped pos";
			}
		}
		else
			LOG(INFO) << "Found vehicle node";

		return outcoords != coords;
	}
}