#include <iostream>
#include <cobra/Instance.hpp>
#include <cobra/Solution.hpp>
#include <cobra/LocalSearch.hpp>
#include <iomanip>
#include <chrono>
#include <cobra/PrettyPrinter.hpp>
#include <cobra/SimulatedAnnealing.hpp>
#include <cobra/Welford.hpp>
#include <filesystem>
#include "bpp.hpp"
#include "routemin.hpp"
#include "RuinAndRecreate.hpp"
#include "arg_parser.hpp"

#ifdef GUI
#include "Renderer.hpp"
#endif

// Available parsers
#define X_PARSER ("X")
#define K_PARSER ("K")
#define Z_PARSER ("Z")


auto get_basename(const std::string& pathname) -> std::string {
    return {std::find_if(pathname.rbegin(), pathname.rend(),
                         [](char c) { return c == '/'; }).base(),
            pathname.end()};
}


auto main(int argc, char* argv[]) -> int {

    auto arg_parser = parse_command_line_arguments(argc, argv);

    const auto outfile = arg_parser.get_outpath() + get_basename(arg_parser.get_instance_path()) + "_seed-"
        + std::to_string(arg_parser.get_seed())+".out";

    std::filesystem::create_directories(arg_parser.get_outpath());

    const auto global_time_begin = std::chrono::high_resolution_clock::now();

    auto rand_engine = std::mt19937(arg_parser.get_seed());

    #ifdef VERBOSE
    auto partial_time_begin = std::chrono::high_resolution_clock::now();
    auto partial_time_end = std::chrono::high_resolution_clock::now();

    std::cout << "Pre-processing the instance.\n";
    partial_time_begin = std::chrono::high_resolution_clock::now();
    #endif

    const auto parser_type = arg_parser.get_parser();

    auto round_costs = true;
    if(parser_type == Z_PARSER || parser_type == K_PARSER) { round_costs = false; }

    auto maybe_instance = parser_type == X_PARSER ?
                          cobra::Instance::make<cobra::XInstanceParser, true>(arg_parser.get_instance_path()) :
                          (parser_type == Z_PARSER ?
                          cobra::Instance::make<cobra::ZKInstanceParser, false>(arg_parser.get_instance_path()) :
                          cobra::Instance::make<cobra::KytojokiInstanceParser, false>(arg_parser.get_instance_path()));


    if (!maybe_instance) {
        std::cout << "Error while parsing the instance '" << arg_parser.get_instance_path() << "'.\n";
        exit(EXIT_FAILURE);
    }

    const auto instance = std::move(maybe_instance.value());

    #ifdef VERBOSE
    partial_time_end = std::chrono::high_resolution_clock::now();
    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(partial_time_end - partial_time_begin).count() << " milliseconds.\n\n";

    std::cout << "Computing mean arc cost.\n";
    partial_time_begin = std::chrono::high_resolution_clock::now();
    #endif

    auto mean_arc_cost = 0.0;
    for(auto i = instance.get_vertices_begin(); i < instance.get_vertices_end()-1; i++) {
        for(auto j = i + 1; j < instance.get_vertices_end(); j++) {
            mean_arc_cost += instance.get_cost(i, j);
        }
    }
    mean_arc_cost /= (instance.get_vertices_num() * (instance.get_vertices_num() - 1) / 2.0);

    #ifdef VERBOSE
    partial_time_end = std::chrono::high_resolution_clock::now();
    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(partial_time_end - partial_time_begin).count() << " milliseconds.\n";
    std::cout << "Mean arc cost value is " << mean_arc_cost << ".\n";

    std::cout << "Computing a greedy upper bound on the n. of routes.\n";
    partial_time_begin = std::chrono::high_resolution_clock::now();
    #endif

    auto kmin = bpp::greedy_first_fit_decreasing(instance);

    #ifdef VERBOSE
    partial_time_end = std::chrono::high_resolution_clock::now();
    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(partial_time_end - partial_time_begin).count() << " milliseconds.\n";
    std::cout << "Around " << kmin << " routes should do the job.\n\n";

    std::cout << "Setting up MOVEGENERATORS data structures.\n";
    partial_time_begin = std::chrono::high_resolution_clock::now();
    #endif

    auto k = arg_parser.get_sparsification_rule_neighbors();
    auto knn_view = cobra::KNeighborsMoveGeneratorsView(instance, k);

    auto views = std::vector<cobra::AbstractMoveGeneratorsView*>();
    views.push_back(&knn_view);

    auto move_generators = cobra::MoveGenerators(instance, views);

    #ifdef VERBOSE
    partial_time_end = std::chrono::high_resolution_clock::now();
    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(partial_time_end - partial_time_begin).count() << " milliseconds.\n";
    const auto tot_arcs = instance.get_vertices_num() * instance.get_vertices_num();
    const auto move_gen_num = move_generators.get_raw_vector().size();
    const auto move_gen_perc = 100.0f * move_gen_num / tot_arcs;
    std::cout << "Using at most " << move_generators.get_raw_vector().size() << " move-generators out of " << tot_arcs << " total arcs ";
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    std::cout << "(approx. " << move_gen_perc << "%):\n";
    std::cout << std::setprecision(10);
    std::cout << std::defaultfloat;
    std::cout << std::setw(10);
    std::cout << knn_view.get_number_of_moves() << " k=" << k << " nearest-neighbors arcs\n";
    std::cout << "\n";

    std::cout << "Setting up LOCALSEARCH data structures.\n";
    partial_time_begin = std::chrono::high_resolution_clock::now();
    #endif

    const auto tolerance = arg_parser.get_tolerance();
    auto rvnd0 = cobra::RandomizedVariableNeighborhoodDescent(instance, move_generators, {
        cobra::E11,cobra::E10,cobra::TAILS,cobra::SPLIT,cobra::RE22B,
        cobra::E22,cobra::RE20,cobra::RE21,cobra::RE22S,cobra::E21,
        cobra::E20,cobra::TWOPT,cobra::RE30,cobra::E30,cobra::RE33B,
        cobra::E33,cobra::RE31,cobra::RE32B,cobra::RE33S,cobra::E31,
        cobra::E32,cobra::RE32S}, rand_engine, tolerance);
    auto rvnd1 = cobra::RandomizedVariableNeighborhoodDescent(instance, move_generators, {
        cobra::EJCH,
    }, rand_engine, tolerance);

    auto local_search = cobra::HierarchicalVariableNeighborhoodDescent(tolerance);
    local_search.append(&rvnd0);
    local_search.append(&rvnd1);

    #ifdef VERBOSE
    partial_time_end = std::chrono::high_resolution_clock::now();
    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(partial_time_end - partial_time_begin).count() << " milliseconds.\n\n";
    #endif

    const auto solution_cache_size = arg_parser.get_solution_cache_size();

    auto solution = cobra::Solution(instance, std::min(instance.get_vertices_num(), solution_cache_size));

    #ifdef VERBOSE
    std::cout << "Running CLARKE&WRIGHT to generate an initial solution.\n";
    partial_time_begin = std::chrono::high_resolution_clock::now();
    #endif

    cobra::Solution::clarke_and_wright(instance, solution, arg_parser.get_cw_lambda(), arg_parser.get_cw_neighbors());

    #ifdef VERBOSE
    partial_time_end = std::chrono::high_resolution_clock::now();
    std::cout << "Done in " << std::chrono::duration_cast<std::chrono::milliseconds>(partial_time_end - partial_time_begin).count() << " milliseconds.\n";
    std::cout << "Initial solution: obj = " << solution.get_cost() << ", n. of routes = " << solution.get_routes_num() << ".\n\n";
    #endif

    if (kmin < solution.get_routes_num()) {

        const auto routemin_iterations = arg_parser.get_routemin_iterations();

        #ifdef VERBOSE
        std::cout << "Running ROUTEMIN heuristic for at most " << routemin_iterations << " iterations.\n";
        std::cout << "Starting solution: obj = " << solution.get_cost() << ", n. of routes = " << solution.get_routes_num() << ".\n";
        partial_time_begin = std::chrono::high_resolution_clock::now();
        #endif

        solution = routemin(instance, solution,rand_engine,move_generators, kmin, routemin_iterations, tolerance);

        #ifdef VERBOSE
        std::cout << "Final solution: obj = " << solution.get_cost() << ", n. routes = " << solution.get_routes_num() << "\n";
        partial_time_end = std::chrono::high_resolution_clock::now();
        std::cout <<"Done in " << std::chrono::duration_cast<std::chrono::seconds>(partial_time_end - partial_time_begin).count() << " seconds.\n\n";
        #endif
    }

    // retrieve the number of core opt iterations or the total algorithm runtime when TIMEBASED is ON
    const auto coreopt_iterations = arg_parser.get_coreopt_iterations();

    auto best_solution = solution;
    #ifdef VERBOSE
    auto best_solution_time = std::chrono::high_resolution_clock::now();
    #endif

    const auto gamma_base = arg_parser.get_gamma_base();
    auto gamma = std::vector<float>(instance.get_vertices_num(), gamma_base);
    auto gamma_counter = std::vector<int>(instance.get_vertices_num(), 0);

    const auto delta = arg_parser.get_delta();
    auto average_number_of_vertices_accessed = cobra::Welford();

    auto gamma_vertices = std::vector<int>();
    for(auto i = instance.get_vertices_begin(); i < instance.get_vertices_end(); i++) {
        gamma_vertices.emplace_back(i);
    }
    move_generators.set_active_percentage(gamma, gamma_vertices);

    auto ruined_customers = std::vector<int>();

    #ifdef VERBOSE
    std::cout << "Running COREOPT for " << coreopt_iterations << " iterations.\n";

    auto welford_rac_before_shaking = cobra::Welford();
    auto welford_rac_after_shaking = cobra::Welford();
    auto welford_local_optima = cobra::Welford();
    auto welford_shaken_solutions = cobra::Welford();
    auto printer = cobra::PrettyPrinter({
        {"%", cobra::PrettyPrinter::Field::Type::INTEGER, 3, " "},
        {"Iterations", cobra::PrettyPrinter::Field::Type::INTEGER, 10, " "},
        {"Objective", cobra::PrettyPrinter::Field::Type::INTEGER, 10, " "},
        {"Routes", cobra::PrettyPrinter::Field::Type::INTEGER, 6, " "},
        {"Found after (s)", cobra::PrettyPrinter::Field::Type::INTEGER, 15, " "},
        {"Iter/s",cobra::PrettyPrinter::Field::Type::REAL, 10, " "},
        #ifdef TIMEBASED
        {"Eta (s)", cobra::PrettyPrinter::Field::Type::INTEGER, 10, " "},
        #else
        {"Eta (s)", cobra::PrettyPrinter::Field::Type::REAL, 10, " "},
        #endif
        {"Gamma", cobra::PrettyPrinter::Field::Type::REAL, 5, " "},
        {"Omega", cobra::PrettyPrinter::Field::Type::REAL, 6, " "},
        {"Temp", cobra::PrettyPrinter::Field::Type::REAL, 6, " "}
    });

    #ifndef TIMEBASED
    auto main_opt_loop_begin_time = std::chrono::high_resolution_clock::now();
    #endif

    auto elapsed_minutes = 0;
    #endif

    #ifdef GUI
    auto renderer = Renderer(instance, solution.get_cost());
    #endif

    auto rr = RuinAndRecreate(instance, rand_engine);

    const auto intensification_lb = arg_parser.get_shaking_lb_factor();
    const auto intensification_ub = arg_parser.get_shaking_ub_factor();

    const auto mean_solution_arc_cost = solution.get_cost() / (static_cast<float>(instance.get_customers_num()) + 2.0f *static_cast<float>(solution.get_routes_num()));

    auto shaking_lb_factor = mean_solution_arc_cost * intensification_lb;
    auto shaking_ub_factor = mean_solution_arc_cost * intensification_ub;

    #ifdef VERBOSE
    std::cout << "Shaking LB = " << shaking_lb_factor << "\n";
    std::cout << "Shaking UB = " << shaking_ub_factor << "\n";
    #endif

    const auto omega_base = std::max(1, static_cast<int>(std::ceil(std::log(instance.get_vertices_num()))));
    auto omega = std::vector<int>(instance.get_vertices_num(), omega_base);
    auto random_choice = std::uniform_int_distribution(0, 1);

    const auto sa_initial_temperature = mean_arc_cost / 10.0f;
    const auto sa_final_temperature = sa_initial_temperature / 100.0f;

    #ifdef TIMEBASED
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - global_time_begin).count();
    auto sa = cobra::TimeBasedSimulatedAnnealing(sa_initial_temperature, sa_final_temperature, rand_engine, coreopt_iterations - elapsed_time);
    #else
    auto sa = cobra::SimulatedAnnealing(sa_initial_temperature, sa_final_temperature, rand_engine, coreopt_iterations);
    #endif


    #ifdef VERBOSE
    std::cout << "Simulated annealing temperature goes from "<< sa_initial_temperature << " to " << sa_final_temperature << ".\n\n";
    #endif

    solution.clear_cache();

    #ifdef TIMEBASED
    for (auto iter = 0; elapsed_time < coreopt_iterations; iter++) {
    #else
    for (auto iter = 0; iter < coreopt_iterations; iter++) {
    #endif

        auto neighbor = solution;

        #ifdef VERBOSE
        if (std::chrono::duration_cast<std::chrono::minutes>(
            std::chrono::high_resolution_clock::now() - global_time_begin).count() >= elapsed_minutes + 5) {
            printer.notify("Optimizing for " + std::to_string(std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - global_time_begin).count()) + " minutes.");
            elapsed_minutes += 5;
        }
        #endif

        const auto walk_seed = rr.apply(neighbor, omega);

        #ifdef GUI
        const auto shaken_solution_cost = neighbor.get_cost();
        #endif

        ruined_customers.clear();
        for (auto i = neighbor.get_cache().begin(); i!=cobra::LRUCache::Entry::dummy_vertex; i = neighbor.get_cache().get_next(i)) {
            ruined_customers.emplace_back(i);
        }

        #ifdef VERBOSE
        welford_rac_after_shaking.update(static_cast<float>(neighbor.get_cache().size()));
        welford_shaken_solutions.update(neighbor.get_cost());
        #endif

        local_search.apply(neighbor);

        #ifdef GUI
        const auto local_optimum_cost = neighbor.get_cost();
        #endif

        average_number_of_vertices_accessed.update(static_cast<float>(neighbor.get_cache().size()));

        #ifdef TIMEBASED
        const auto iter_per_second = static_cast<float>(iter+1) / (static_cast<float>(elapsed_time) + 0.01f);
        const auto remaining_time = coreopt_iterations - elapsed_time;
        const auto estimated_remaining_iter = iter_per_second * remaining_time;
        const auto expected_total_iterations_num = iter+1 + estimated_remaining_iter;

        const auto max_non_improving_iterations = static_cast<int>(std::ceil(delta * static_cast<float>(expected_total_iterations_num) * static_cast<float>(average_number_of_vertices_accessed.get_mean()) /static_cast<float>(instance.get_vertices_num())));

        #else
        auto max_non_improving_iterations = static_cast<int>(std::ceil(delta * static_cast<float>(coreopt_iterations) * static_cast<float>(average_number_of_vertices_accessed.get_mean()) / static_cast<float>(instance.get_vertices_num())));
        #endif

        #ifdef GUI
        if(iter % 100 == 0) { renderer.draw(best_solution, neighbor.get_cache(),move_generators); }
        #endif

        #ifdef VERBOSE
        welford_rac_before_shaking.update(static_cast<float>(neighbor.get_cache().size()));
        welford_local_optima.update(neighbor.get_cost());
        #endif

        if (neighbor.get_cost() < best_solution.get_cost()) {

            best_solution = neighbor;
            #ifdef VERBOSE
            best_solution_time = std::chrono::high_resolution_clock::now();
            #endif

            gamma_vertices.clear();
            for (auto i = neighbor.get_cache().begin(); i!=cobra::LRUCache::Entry::dummy_vertex; i = neighbor.get_cache().get_next(i)) {
                gamma[i] = gamma_base;
                gamma_counter[i] = 0;
                gamma_vertices.emplace_back(i);
            }
            move_generators.set_active_percentage(gamma, gamma_vertices);

            #ifdef VERBOSE
            welford_local_optima.reset();
            welford_local_optima.update(neighbor.get_cost());
            welford_shaken_solutions.reset();
            welford_shaken_solutions.update(neighbor.get_cost());
            #endif

        } else {

            for (auto i = neighbor.get_cache().begin(); i!=cobra::LRUCache::Entry::dummy_vertex; i = neighbor.get_cache().get_next(i)) {
                gamma_counter[i]++;
                if (gamma_counter[i] >= max_non_improving_iterations) {
                    gamma[i] = std::min(gamma[i] * 2.0f, 1.0f);
                    gamma_counter[i] = 0;
                    gamma_vertices.clear();
                    gamma_vertices.emplace_back(i);
                    move_generators.set_active_percentage(gamma, gamma_vertices);
                }
            }

        }

        const auto seed_shake_value = omega[walk_seed];

        if (neighbor.get_cost() > shaking_ub_factor + solution.get_cost()) {
            for (auto i : ruined_customers) {
                if (omega[i] > seed_shake_value - 1) {
                    omega[i]--;
                }
            }
        } else if (neighbor.get_cost() >= solution.get_cost() && neighbor.get_cost() < solution.get_cost() + shaking_lb_factor) {
            for (auto i : ruined_customers) {
                if (omega[i] < seed_shake_value + 1) {
                    omega[i]++;
                }
            }
        }  else  {
            for(auto i : ruined_customers) {
                if(random_choice(rand_engine)) {
                    if (omega[i] > seed_shake_value - 1) {
                        omega[i]--;
                    }
                } else {
                    if (omega[i] < seed_shake_value + 1) {
                        omega[i]++;
                    }
                }
            }
        }


        #ifdef TIMEBASED
        elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - global_time_begin).count();
        if (sa.accept(solution, neighbor, elapsed_time)) {
        #else
        if (sa.accept(solution, neighbor)) {
        #endif
            solution = neighbor;
            if(!round_costs) { solution.recompute_costs(); } // avoid too many rounding errors get summed during LS
            solution.clear_cache();
            const auto updated_mean_solution_arc_cost = solution.get_cost() / (static_cast<float>(instance.get_customers_num()) + 2.0f * static_cast<float>(solution.get_routes_num()));
            shaking_lb_factor = updated_mean_solution_arc_cost * intensification_lb;
            shaking_ub_factor = updated_mean_solution_arc_cost * intensification_ub;
        }

        sa.decrease_temperature();

        #ifdef GUI
        renderer.add_trajectory_point(shaken_solution_cost, local_optimum_cost, solution.get_cost(), best_solution.get_cost());
        #endif

        #ifdef VERBOSE
        partial_time_end = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(partial_time_end - partial_time_begin).count() > 1) {

            #ifdef TIMEBASED
            const auto progress = 100.0f*elapsed_time/coreopt_iterations;
            const auto estimated_rem_time = remaining_time;
            #else
            const auto progress = 100.0f*(iter + 1.0f)/coreopt_iterations;
            const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - main_opt_loop_begin_time).count();
            const auto iter_per_second = static_cast<float>(iter + 1)/(static_cast<float>(elapsed_seconds) + 0.01f);
            const auto remaining_iter = coreopt_iterations - iter;
            const auto estimated_rem_time = static_cast<float>(remaining_iter)/iter_per_second;
            #endif

            auto gamma_mean = 0.0f;
            for (auto i = instance.get_vertices_begin(); i < instance.get_vertices_end(); i++) { gamma_mean += gamma[i]; }
            gamma_mean = (gamma_mean/static_cast<float>(instance.get_vertices_num()));


            auto omega_mean = 0.0f;
            for (auto i = instance.get_customers_begin(); i < instance.get_customers_end(); i++) {
                omega_mean += omega[i];
            }
            omega_mean /= static_cast<float>(instance.get_customers_num());


            printer.print(progress, iter + 1,
                          best_solution.get_cost(),
                          best_solution.get_routes_num(),
                          std::chrono::duration_cast<std::chrono::seconds>(best_solution_time - global_time_begin).count(),
                          iter_per_second,
                          estimated_rem_time,
                          gamma_mean,
                          omega_mean,
                          #ifdef TIMEBASED
                          sa.get_temperature(elapsed_time)
                          #else
                          sa.get_temperature()
                          #endif
            );

            partial_time_begin = std::chrono::high_resolution_clock::now();

        }
        #endif

    }

    const auto global_time_end = std::chrono::high_resolution_clock::now();

    #ifdef VERBOSE
    std::cout << "\n";
    std::cout << "Best solution found:\n";
    std::cout << "obj = " << best_solution.get_cost() << ", n. routes = " << best_solution.get_routes_num() << ", found after = " << std::chrono::duration_cast<std::chrono::seconds>(best_solution_time - global_time_begin).count() << " seconds ";
    std::cout << "(" << std::chrono::duration_cast<std::chrono::milliseconds>(best_solution_time - global_time_begin).count() << " milliseconds).\n";

    std::cout << "\n";
    std::cout << "Run completed in " << std::chrono::duration_cast<std::chrono::seconds>(global_time_end - global_time_begin).count() << " seconds ";
    std::cout << "(" << std::chrono::duration_cast<std::chrono::milliseconds>(global_time_end - global_time_begin).count() << " milliseconds).\n";
    #endif

    auto out_stream = std::ofstream(outfile);
    out_stream << std::setprecision(10);
    out_stream << best_solution.get_cost() << "\t" << std::chrono::duration_cast<std::chrono::seconds>(global_time_end - global_time_begin).count() << "\n";
    cobra::Solution::store_to_file(instance, best_solution, arg_parser.get_outpath() + get_basename(arg_parser.get_instance_path()) + "_seed-" + std::to_string(arg_parser.get_seed())+".vrp.sol");

    #ifdef VERBOSE
    std::cout << "\n";
    std::cout << "Results stored in\n";
    std::cout << " - " << outfile << "\n";
    std::cout << " - " << arg_parser.get_outpath() + get_basename(arg_parser.get_instance_path()) + "_seed-" + std::to_string(arg_parser.get_seed())+".vrp.sol" << "\n";
    #endif

    return EXIT_SUCCESS;

}
