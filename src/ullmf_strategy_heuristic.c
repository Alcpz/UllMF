/*
 * Copyright (c) 2018 Universidad de La Laguna <cap@pcg.ull.es>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * File: ullmf_strategy_heuristic.c
 * Date: 23 jul. 2018
 * Author: Alberto Cabrera <Alberto.Cabrera@ull.edu.es>
 */

#include "ullmf_strategy_heuristic.h"
#include "ullmf_calibration.h"
#include "debug.h"

#include <math.h>
#include <float.h>
#include <string.h>

static const double epsilon = 0.0005;

double _ullmf_evalue_sum(ullmf_calibration_t* calib,
					     int* candidate_counts, double* resource_ratios) {
	double evaluation = 0;
	for (int i = 0; i < calib->num_procs; i++) {
		evaluation += candidate_counts[i] * resource_ratios[i];
	}
	return evaluation;
}

double _ullmf_evalue_max(ullmf_calibration_t* calib,
					     int* candidate_counts, double* resource_ratios) {
	double evaluation = 0;
	for (int i = 0; i < calib->num_procs; i++) {
		evaluation += candidate_counts[i] * resource_ratios[i];
	}
	return evaluation;
}

static bool is_movement_legal(double current_ratio, double ratio_step, int direction) {
    if ((current_ratio + ratio_step * direction) >= 0
            && (current_ratio + ratio_step * direction) <= 1)
        return true;
    return false;
}

static double get_resource_ratio(double resource_consumption, int counts) {
    if (resource_consumption < epsilon) // <= 0
    	return 0;
    else
    	return resource_consumption / (double) counts;
}

static ullmf_workload_t * move_workload(ullmf_calibration_t* calib, int fromto, int direction) {
	ullmf_strategy_heuristic_t * heuristic = (ullmf_strategy_heuristic_t *) calib->strategy;

	double to_move = heuristic->search_distance;
	double * new_ratios;
	size_t memsize = calib->num_procs * sizeof(double);
	memcpy(new_ratios, calib->workload->ratios, memsize);

	new_ratios[fromto] += to_move * direction;
	double remaining_movement = to_move;
	while (remaining_movement > epsilon && remaining_movement < -epsilon) {
		double current_movement = remaining_movement / (calib->num_procs - 1);
		for (int i = 0; i < calib->num_procs; i++) {
			if (i == fromto)
				continue;
			if (is_movement_legal(new_ratios[i], current_movement, -direction)) {
				new_ratios[i] += current_movement * -direction;
				remaining_movement -= current_movement;
			}
		}
	}

	ullmf_distribution_t * new_distribution = _new(Distribution, calib->num_procs, new_ratios);
	ullmf_workload_t * new_workload = calib->workload->new_from_distribution(calib->workload, new_distribution);
	_delete(new_distribution);
	free(new_ratios);
	return new_workload;
}

static int generate_distributions(ullmf_calibration_t* calib, ullmf_workload_t** candidates) {
	ullmf_strategy_heuristic_t * heuristic = (ullmf_strategy_heuristic_t *) calib->strategy;
	int num_candidates = calib->num_procs * 2; // TODO substitution for parametric value heuristic size
											   // should be included inside heuristic constructor
	int direction = 1;
	candidates = malloc(num_candidates * sizeof(candidates));
	for (int i = 0; i < num_candidates; i++) {
		int process = i % calib->num_procs;
		if (is_movement_legal(calib->workload->ratios[process], heuristic->search_distance, direction)) {
			candidates[i] = move_workload(calib, process, direction);
		}
		direction = -direction;
	}
	return num_candidates;
}

static void free_distributions(int num_candidates, ullmf_workload_t** candidates) {
	for (int i = 0; i < num_candidates; i++)
		_delete(candidates[i]);
	free(candidates);
}

static ullmf_distribution_t * heuristic_search(ullmf_calibration_t* calib) {
	ullmf_strategy_heuristic_t * heuristic = (ullmf_strategy_heuristic_t *) calib->strategy;

	// Calculate resources per unit of work
	double * resource_ratios = calloc(calib->num_procs, sizeof(double));
    for (int i = 0; i < calib->num_procs; i++)
    	resource_ratios[i] = get_resource_ratio(calib->measurements[i], calib->workload->counts[i]);

    // Generate heuristic population of candidates
    ullmf_workload_t ** candidates;
    int num_candidates = generate_distributions(calib->workload, candidates);

    // Evaluate heuristic population
    ullmf_distribution_t * best_candidate = _new(Distribution, calib->num_procs, calib->workload->ratios);
    int * best_counts = calib->workload->counts;
    double best_consumption = heuristic->evalue_workload_distribution(calib, calib->workload->counts, resource_ratios);
    double candidate_consumption;
    for (int i = 0; i < num_candidates; i++) {
    	candidate_consumption = heuristic->evalue_workload_distribution(calib, candidates[i], resource_ratios);
    	if (candidate_consumption < best_consumption) {
    		candidate_consumption = best_consumption;
    		best_candidate->set_ratios(best_candidate, candidates[i]->ratios);
    	}
    }

    free_distributions(num_candidates, candidates);
    free(resource_ratios);
	return best_candidate;
}

int _ullmf_heuristic_calibrate(ullmf_calibration_t* calib) {
	if (calib->strategy->mdevice->measuring) // Energy measurements require time
		return ULLMF_TAG_CALIBRATED;

	ullmf_strategy_heuristic_t* heuristic = (ullmf_strategy_heuristic_t*) calib->strategy;
	if (heuristic->search_distance < heuristic->search_threshold) {
		//Probability
		if (!heuristic->tried_inversion) {
			heuristic->tried_inversion = true;
		} else {
			double reset = (double)(random() % 10000) / 100; // TODO change to int logic
      if (reset < heuristic->reset_prob)
				heuristic->search_distance = heuristic->reset_search_distance;
				heuristic->reset_probability = heuristic->initial_reset_probability;
				// TODO Inversion on last reset
			} else {
				heuristic->reset_probability += heuristic->reset_probability_increment;
				return ULLMF_TAG_CALIBRATED;
			}
		}
	}

	if (calib->strategy->best_candidate != 0)
		_delete(calib->strategy->best_candidate);
	calib->strategy->best_candidate = heuristic_search(calib);

	return ULLMF_TAG_RECALIBRATING;
}