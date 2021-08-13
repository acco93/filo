//
// Created by acco on 7/30/20.
//

#ifndef FILO__ARG_PARSER_HPP_
#define FILO__ARG_PARSER_HPP_

/* Default parameters */
#define DEFAULT_OUTPATH ("./")
#define DEFAULT_PARSER ("X")
#define DEFAULT_SOLUTION_CACHE_HISTORY (50)
#define DEFAULT_CW_LAMBDA (1.0f)
#define DEFAULT_CW_NEIGHBORS (100)
#define DEFAULT_ROUTEMIN_ITERATIONS (1000)
#ifdef TIMEBASED
#define DEFAULT_COREOPT_ITERATIONS (60)
#else
#define DEFAULT_COREOPT_ITERATIONS (100000)
#endif
#define DEFAULT_SPARSIFICATION_RULE1_NEIGHBORS (25)
#define DEFAULT_SPARSIFICATION_FACTOR (0.25f)
#define DEFAULT_SPARSIFICATION_MULTIPLIER (0.50f)
#define DEFAULT_SHAKING_LB_FACTOR (0.375f)
#define DEFAULT_SHAKING_UB_FACTOR (0.85f)
#define DEFAULT_TOLERANCE (0.01f)
#define DEFAULT_SEED (0)

/* Tokens */
#define TOKEN_OUTPATH ("--outpath")
#define TOKEN_PARSER ("--parser")
#define TOKEN_TOLERANCE ("--tolerance")
#define TOKEN_SPARSIFICATION_RULE1_NEIGHBORS ("--granular-neighbors")
#define TOKEN_SOLUTION_CACHE_HISTORY ("--cache")
#define TOKEN_ROUTEMIN_ITERATIONS ("--routemin-iterations")
#ifdef TIMEBASED
#define TOKEN_COREOPT_ITERATIONS ("--time")
#else
#define TOKEN_COREOPT_ITERATIONS ("--coreopt-iterations")
#endif
#define TOKEN_SPARSIFICATION_FACTOR ("--granular-gamma-base")
#define TOKEN_SPARSIFICATION_MULTIPLIER ("--granular-delta")
#define TOKEN_SHAKING_LB_FACTOR ("--shaking-lower-bound")
#define TOKEN_SHAKING_UB_FACTOR ("--shaking-upper-bound")
#define TOKEN_SEED ("--seed")
#define TOKEN_HELP ("--help")

class Parameters {

    std::string instance_path = "";

 private:
    std::string outpath = DEFAULT_OUTPATH;
    std::string parser = DEFAULT_PARSER;
    float tolerance = DEFAULT_TOLERANCE;
    int solution_cache_history = DEFAULT_SOLUTION_CACHE_HISTORY;
    float cw_lambda = DEFAULT_CW_LAMBDA;
    int cw_neighbors = DEFAULT_CW_NEIGHBORS;
    int routemin_iterations = DEFAULT_ROUTEMIN_ITERATIONS;
    int coreopt_iterations = DEFAULT_COREOPT_ITERATIONS;
    int sparsification_rule_neighbors = DEFAULT_SPARSIFICATION_RULE1_NEIGHBORS;
    float gamma_base = DEFAULT_SPARSIFICATION_FACTOR;
    float delta = DEFAULT_SPARSIFICATION_MULTIPLIER;
    float shaking_lb_factor = DEFAULT_SHAKING_LB_FACTOR;
    float shaking_ub_factor = DEFAULT_SHAKING_UB_FACTOR;
    int seed = DEFAULT_SEED;

 public:

    Parameters(std::string instance_path_) : instance_path(instance_path_) {}

    int get_solution_cache_size() const {
        return solution_cache_history;
    }
    float get_cw_lambda() const {
        return cw_lambda;
    }
    int get_cw_neighbors() const {
        return cw_neighbors;
    }
    int get_routemin_iterations() const {
        return routemin_iterations;
    }
    int get_coreopt_iterations() const {
        return coreopt_iterations;
    }
    int get_sparsification_rule_neighbors() const {
        return sparsification_rule_neighbors;
    }
    float get_gamma_base() const {
        return gamma_base;
    }
    float get_delta() const {
        return delta;
    }
    float get_shaking_lb_factor() const {
        return shaking_lb_factor;
    }
    float get_shaking_ub_factor() const {
        return shaking_ub_factor;
    }
    float get_tolerance() const {
        return tolerance;
    }
    std::string get_instance_path() const { return instance_path; }
    std::string get_outpath() const { return outpath; }
    std::string get_parser() const { return parser; }
    int get_seed() const { return seed; }

    void set(std::string key, std::string value) {

        if(key == TOKEN_OUTPATH) {
            outpath = value;
            if(outpath.back() != '/') {
                outpath += '/';
            }
        } else if (key == TOKEN_PARSER) {
            parser = value;
        } else if (key == TOKEN_TOLERANCE) {
            tolerance = std::stof(value);
        } else if (key == TOKEN_SPARSIFICATION_RULE1_NEIGHBORS) {
            sparsification_rule_neighbors = std::stoi(value);
        } else if (key == TOKEN_SOLUTION_CACHE_HISTORY) {
            solution_cache_history = std::stoi(value);
        } else if (key == TOKEN_ROUTEMIN_ITERATIONS) {
            routemin_iterations = std::stoi(value);
        } else if (key == TOKEN_COREOPT_ITERATIONS){
            coreopt_iterations = std::stoi(value);
        } else if (key == TOKEN_SPARSIFICATION_FACTOR) {
            gamma_base = std::stof(value);
        } else if (key == TOKEN_SPARSIFICATION_MULTIPLIER){
            delta = std::stof(value);
        } else if (key == TOKEN_SHAKING_LB_FACTOR){
            shaking_lb_factor = std::stof(value);
        } else if (key == TOKEN_SHAKING_UB_FACTOR) {
            shaking_ub_factor = std::stof(value);
        } else if (key == TOKEN_SEED) {
            seed = std::stoi(value);
        } else {
            std::cout << "Error: unknown argument '" << key <<"'. Try --help for more information.\n";
            exit(EXIT_SUCCESS);
        }

    }


};

void print_help() {

    std::cout << "Usage: filo <path-to-instance> [OPTIONS]\n\n";

    std::cout << "Available options\n";

    std::cout << TOKEN_OUTPATH << " STRING\t\tOutput directory (default: " << DEFAULT_OUTPATH << ")\n";
    std::cout << TOKEN_PARSER << " STRING\t\t\tParser type, it can be X, K or Z (default: " << DEFAULT_PARSER << ")\n";
    std::cout << TOKEN_TOLERANCE << " FLOAT\t\tFloating point tolerance (default: " << DEFAULT_TOLERANCE << ")\n";
    std::cout << TOKEN_SPARSIFICATION_RULE1_NEIGHBORS << " INT\tNeighbors per vertex in granular neighborhoods (default: "<<DEFAULT_SPARSIFICATION_RULE1_NEIGHBORS << ")\n";
    std::cout << TOKEN_SOLUTION_CACHE_HISTORY << " INT\t\t\tSelective cache dimension (default: " << DEFAULT_SOLUTION_CACHE_HISTORY <<")\n";
    std::cout << TOKEN_ROUTEMIN_ITERATIONS << " INT\tMax route minimization iterations (default: " << DEFAULT_ROUTEMIN_ITERATIONS << ")\n";
    #ifdef TIMEBASED
    std::cout << TOKEN_COREOPT_ITERATIONS << " INT\t\t\tRuntime in seconds (default: " << DEFAULT_COREOPT_ITERATIONS << ")\n";
    #else
    std::cout << TOKEN_COREOPT_ITERATIONS << " INT\tCore optimization iterations (default: " << DEFAULT_COREOPT_ITERATIONS << ")\n";
    #endif
    std::cout << TOKEN_SPARSIFICATION_FACTOR << " FLOAT\tInitial sparsification factor gamma base (default: " << DEFAULT_SPARSIFICATION_FACTOR << ")\n";
    std::cout << TOKEN_SPARSIFICATION_MULTIPLIER << " FLOAT\t\tGranular reduction factor delta (default: " << DEFAULT_SPARSIFICATION_MULTIPLIER << ")\n";
    std::cout << TOKEN_SHAKING_LB_FACTOR << " FLOAT\tShaking lower bound factor (default: " << DEFAULT_SHAKING_LB_FACTOR << ")\n";
    std::cout << TOKEN_SHAKING_UB_FACTOR << " FLOAT\tShaking upper bound factor (default: " << DEFAULT_SHAKING_UB_FACTOR << ")\n";
    std::cout << TOKEN_SEED << " INT\t\t\tSeed (default: " << DEFAULT_SEED << ")\n";

    std::cout << "\nReport bugs to luca.accorsi4 and the domain is unibo.it\n";

}

Parameters parse_command_line_arguments(int argc, char* argv[]) {

    if(argc == 1) {
        std::cout << "Missing input instance.\n\n";
        print_help();
        exit(EXIT_FAILURE);
    }

    if(argc == 2 &&  std::string(argv[1]) == TOKEN_HELP) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    auto parameters = Parameters(std::string(argv[1]));

    for(auto n = 2; n < argc; n+=2) {

        auto token = std::string(argv[n]);

        if(token == TOKEN_HELP) {
            print_help();
            exit(EXIT_SUCCESS);
        }

        if(n + 1 >= argc) {
            std::cout << "Missing value for '" << token << "'.\n\n";
            print_help();
            exit(EXIT_FAILURE);
        }
        auto value = std::string(argv[n+1]);



        parameters.set(token, value);

    }

    return parameters;

}

#endif //FILO__ARG_PARSER_HPP_
