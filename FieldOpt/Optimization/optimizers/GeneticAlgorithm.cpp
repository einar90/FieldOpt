/******************************************************************************
   Created by einar on 2/27/17.
   Copyright (C) 2017 Einar J.M. Baumann <einar.baumann@gmail.com>

   This file is part of the FieldOpt project.

   FieldOpt is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   FieldOpt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FieldOpt.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "GeneticAlgorithm.h"
#include "Utilities/math.hpp"
#include <math.h>

namespace Optimization {
namespace Optimizers {

GeneticAlgorithm::GeneticAlgorithm(Settings::Optimizer *settings,
                                   Case *base_case,
                                   Model::Properties::VariablePropertyContainer *variables,
                                   Reservoir::Grid::Grid *grid) : Optimizer(settings, base_case, variables, grid) {
    max_generations_ = 50;
    n_ints_ = variables->DiscreteVariableSize();
    n_floats_ = variables->ContinousVariableSize();
    generation_ = 0;
    population_size_ = 20;
    p_crossover_ = 0.8;
    p_mutation_ = 0.01;
    location_parameter_ = 0.0;
    scale_real_ = 0.15;
    scale_int_ = 0.2;
    power_real_ = 10.0;
    power_int_= 4.0;
    float_lower_bounds_ = Eigen::VectorXd::Zero(n_floats_);
    float_upper_bounds_ = Eigen::VectorXd::Zero(n_floats_);
    float_lower_bounds_.fill(1000.0);
    int_lower_bounds_ = Eigen::VectorXd::Zero(n_ints_);
    int_upper_bounds_ = Eigen::VectorXd::Zero(n_ints_);
    int_upper_bounds_.fill(60.0);

    assert(population_size_ % 2 == 0); // Ensure that the population size is an even number.
    // todo: Generate initial chromosomes and add the cases to the queue
}
Optimizer::TerminationCondition GeneticAlgorithm::IsFinished() {
    if (case_handler_->EvaluatedCases().size() > max_evaluations_ ||
        generation_ > max_generations_)
        return TerminationCondition::MAX_EVALS_REACHED;
    else
        return TerminationCondition::NOT_FINISHED;
}
void GeneticAlgorithm::handleEvaluatedCase(Case *c) {
    return; // It shouldn't be necessary to do anything here.
}
void GeneticAlgorithm::iterate() {
    auto mating_pool = tournamentSelection();
    auto offspring = laplaceCrossover(mating_pool);
    offspring = powerMutation(offspring);
    population_ = offspring;
    for (auto c : population_) {
        c.createNewCase();
        case_handler_->AddNewCase(c.c);
    }
    generation_++;
}
vector<GeneticAlgorithm::Chromosome> GeneticAlgorithm::tournamentSelection() {
    vector<Chromosome> mating_pool;
    while (mating_pool.size() < population_size_) {
        auto p1 = population_[random_integer(0, population_size_-1)];
        auto p2 = population_[random_integer(0, population_size_-1)];
        if (isBetter(p1.c, p2.c))
            mating_pool.push_back(p1);
        else
            mating_pool.push_back(p2);
    }
    return vector<Chromosome>();
}
vector<GeneticAlgorithm::Chromosome> GeneticAlgorithm::laplaceCrossover(vector<Chromosome> mating_pool) {
    vector<float> us = random_floats(population_size_);
    vector<float> rs = random_floats(population_size_);
    vector<float> ps = random_floats(population_size_);
    vector<Chromosome> offspring;

    for (int i = 0; i < population_size_; i += 2) {
        if (ps[i] > p_crossover_) { // Not doing crossover
            offspring.push_back(mating_pool[i]);
            offspring.push_back(mating_pool[i+1]);
            continue;
        } // Doing crossover
        float beta_real, beta_int;
        if (rs[i] <= 0.5) {
            beta_real = location_parameter_ - scale_real_ * log(us[i]);
            beta_int = location_parameter_ - scale_int_ * log(us[i]);
        }
        else {
            beta_real = location_parameter_ + scale_real_ * log(us[i]);
            beta_int = location_parameter_ + scale_int_ * log(us[i]);
        }

        Chromosome p1 = mating_pool[i]; // Parent 1
        Chromosome p2 = mating_pool[i+1]; // Parent 2
        Chromosome o1, o2; // Offspring 1 and 2

        o1.rea_vars = p1.rea_vars + beta_real*(p1.rea_vars - p2.rea_vars).cwiseAbs();
        o1.int_vars = p1.int_vars + beta_int*(p1.int_vars - p2.int_vars).cwiseAbs();
        offspring.push_back(o1);

        o2.rea_vars = p2.rea_vars + beta_real*(p1.rea_vars - p2.rea_vars).cwiseAbs();
        o2.int_vars = p2.int_vars + beta_int*(p1.int_vars - p2.int_vars).cwiseAbs();
        offspring.push_back(o2);
    }
    return offspring;
}
vector<GeneticAlgorithm::Chromosome> GeneticAlgorithm::powerMutation(vector<Chromosome> chromosomes) {
    vector<float> ps = random_floats(population_size_);
    vector<float> ss_int = random_floats(population_size_);
    vector<float> ss_float = random_floats(population_size_);
    vector<float> rs = random_floats(population_size_);
    for (int i = 0; i < population_size_; ++i) {
        if (rs[i] <= p_mutation_) {
            ss_int[i] = pow(ss_int[i], power_int_);
            ss_float[i] = pow(ss_float[i], power_real_);
            Eigen::VectorXd t_int = (chromosomes[i].int_vars - int_lower_bounds_) * (int_upper_bounds_ - chromosomes[i].int_vars).inverse();
            Eigen::VectorXd t_rea = (chromosomes[i].rea_vars - float_lower_bounds_) * (float_upper_bounds_ - chromosomes[i].rea_vars).inverse();
            if (t_int[i] < rs[i]) {
                chromosomes[i].int_vars = chromosomes[i].int_vars - ss_int[i] * (chromosomes[i].int_vars - int_lower_bounds_);
                chromosomes[i].rea_vars = chromosomes[i].rea_vars - ss_float[i] * (chromosomes[i].rea_vars - float_lower_bounds_);
            }
            else {
                chromosomes[i].int_vars = chromosomes[i].int_vars + ss_int[i] * (int_upper_bounds_ - chromosomes[i].int_vars);
                chromosomes[i].rea_vars = chromosomes[i].rea_vars + ss_float[i] * (float_upper_bounds_ - chromosomes[i].rea_vars);
            }
        }
    }
    return chromosomes;
}

GeneticAlgorithm::Chromosome::Chromosome(Case *c) {
    this->c = c;
    int_vars = c->GetIntegerVarVector().cast<double>();
    rea_vars = c->GetRealVarVector();
}
void GeneticAlgorithm::Chromosome::truncateInts() {
    vector<float> rs = random_floats(int_vars.size());
    for (int i = 0; i < int_vars.size(); ++i) {
        float int_part = floor(int_vars[i]);
        if (rs[i] >= 0.5)
            int_part += 1;
        int_vars[i] = int_part;
    }
}
void GeneticAlgorithm::Chromosome::createNewCase() {
    truncateInts();
    Case *new_case = new Case(c);
    new_case->SetRealVarValues(rea_vars);
    new_case->SetIntegerVarValues(int_vars.cast<int>());
    c = new_case;
}
}
}
