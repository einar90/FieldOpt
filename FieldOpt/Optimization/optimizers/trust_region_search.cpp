#include "trust_region_search.h"

namespace Optimization {
    namespace Optimizers {

        TrustRegionSearch::TrustRegionSearch(::Settings::Optimizer *settings, Case *base_case,
                                     Model::Properties::VariablePropertyContainer *variables,
                                     Reservoir::Grid::Grid *grid)
                : Optimizer(settings, base_case, variables, grid)
        {
            radius_ = settings->parameters().initial_step_length;
            minimum_radius_ = settings->parameters().minimum_step_length;
        }

        void TrustRegionSearch::step()
        {
            applyNewTentativeBestCase();
        }

        void TrustRegionSearch::contract()
        {
            radius_ = radius_/2.0;
        }

        void TrustRegionSearch::expand()
        {
            radius_ = radius_*2.0;
        }

        void TrustRegionSearch::perturb()
        {
            QList<Case *> perturbations = QList<Case *>();
            for (QUuid id : tentative_best_case_->integer_variables().keys())
                perturbations.append(tentative_best_case_->Perturb(id, Case::SIGN::PLUSMINUS, radius_));
            for (QUuid id : tentative_best_case_->real_variables().keys())
                perturbations.append(tentative_best_case_->Perturb(id, Case::SIGN::PLUSMINUS, radius_));
            for (Case *c : perturbations) {
                constraint_handler_->SnapCaseToConstraints(c);
            }
            case_handler_->AddNewCases(perturbations);
        }

        Optimizer::TerminationCondition TrustRegionSearch::IsFinished()
        {
            if (case_handler_->EvaluatedCases().size() >= max_evaluations_)
                return MAX_EVALS_REACHED;
            else if (radius_ < minimum_radius_)
                return MINIMUM_STEP_LENGTH_REACHED;
            else return NOT_FINISHED; // The value of not finished is 0, which evaluates to false.
        }

        void TrustRegionSearch::iterate()
        {
            /* At the first iteration we initialze the PolyModel
             * object with the initial point
             */
            if (iteration_ == 0) {
                QList<Eigen::VectorXd> points;
                Eigen::VectorXd initial_point;
                initial_point << 1,3,2; //TODO: Need to get this from runner input.
                QList<double> fvalues;
                if(tentative_best_case_->objective_function_value()) {
                    fvalues.append(tentative_best_case_->objective_function_value()); //TODO: Either append correct value or leave empty and calculate later.
                }
                points.append(initial_point);
                polymodel_ = PolyModel(points,fvalues, 3, 2);
                polymodel_.complete_points();

                // The set of points has been completed.
                //TODO: Call runner to get objective function values of the set of points.
            }
            // If we found a better point we move the center of the trust region
            else if (betterCaseFoundLastEvaluation()) {
                step();
                perturb();
            }
            else {
                contract();
                perturb();
            }
            case_handler_->ClearRecentlyEvaluatedCases();
        }

        QString TrustRegionSearch::GetStatusStringHeader() const
        {
            return QString("%1,%2,%3,%4,%5,%6,%7")
                    .arg("Iteration")
                    .arg("EvaluatedCases")
                    .arg("QueuedCases")
                    .arg("RecentlyEvaluatedCases")
                    .arg("TentativeBestCaseID")
                    .arg("TentativeBestCaseOFValue")
                    .arg("StepLength");
        }

        QString TrustRegionSearch::GetStatusString() const
        {
            return QString("%1,%2,%3,%4,%5,%6,%7")
                    .arg(iteration_)
                    .arg(nr_evaluated_cases())
                    .arg(nr_queued_cases())
                    .arg(nr_recently_evaluated_cases())
                    .arg(tentative_best_case_->id().toString())
                    .arg(tentative_best_case_->objective_function_value())
                    .arg(radius_);
        }

    }}