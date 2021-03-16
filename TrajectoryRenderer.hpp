//
// Created by acco on 4/28/20.
//

#ifndef FILO__TRAJECTORY_RENDERER_HPP_
#define FILO__TRAJECTORY_RENDERER_HPP_

#include <cobra/Solution.hpp>
#include <GLFW/glfw3.h>

class TrajectoryRenderer {

    GLFWwindow* window = nullptr;

    float initial_cost;

    float worst_gap = 0.0f;
    float best_gap = std::numeric_limits<float>::max();

    std::vector<float> shaken_solutions_trajectory;
    std::vector<float> local_optima_trajectory;
    std::vector<float> current_trajectory;
    std::vector<float> best_trajectory;

 public:

    TrajectoryRenderer(float initial_cost_) : initial_cost(initial_cost_) {

        if(!glfwInit()) {
            std::cerr << "Cannot successfully execute 'glfwInit'\n";
            abort();
        }

        window = glfwCreateWindow(960, 420, "FILO", nullptr, nullptr);

        if(!window) {
            std::cerr << "Cannot successfully execute 'glfwCreateWindow'\n";
            abort();
        }

        glfwSetWindowCloseCallback(window, [](GLFWwindow*) -> void { exit(1); });

        glfwMakeContextCurrent(window);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);

        glfwPollEvents();

    }

    void add(float shaken_solution_cost, float local_optimum_cost, float current_solution_cost, float best_solution_cost) {

        const auto shaken_solution_gap = 100.0f * (shaken_solution_cost - initial_cost) / initial_cost;
        const auto local_optimum_gap = 100.0f * (local_optimum_cost - initial_cost) / initial_cost;
        const auto current_solution_gap = 100.0f * (current_solution_cost - initial_cost) / initial_cost;
        shaken_solutions_trajectory.emplace_back(shaken_solution_gap);
        local_optima_trajectory.emplace_back(local_optimum_gap);
        current_trajectory.emplace_back(current_solution_gap);

        const auto max = std::max(shaken_solution_gap, local_optimum_gap);

        if(max > worst_gap) {
            worst_gap = max;
        }

        best_gap = 100.0f * (best_solution_cost - initial_cost) / initial_cost;

        best_trajectory.emplace_back(best_gap);

    }

    void render() {

        glfwMakeContextCurrent(window);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0f, shaken_solutions_trajectory.size()+1, std::min(0.0f, std::round(best_gap-0.5f)), worst_gap, -1, 1);

        glMatrixMode(GL_MODELVIEW);

        glClearColor(0.20, 0.20, 0.20, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glColor4f(1.0f,1.0f,1.0f, 0.25f);
        glPushAttrib(GL_ENABLE_BIT);
        glLineStipple(5, 0xAAAA);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINES);
        for(auto i = std::min(static_cast<int>(std::round(best_gap-0.5f)), 1); i <= worst_gap+1; i++) {
            glVertex2f(0, i);
            glVertex2d(best_trajectory.size(), i);
        }
        glEnd();
        glPopAttrib();

        glColor4f(1.0f,0.0f,0.0f, 0.50f);

        glBegin (GL_POINTS);
        for(auto x = 0u; x < shaken_solutions_trajectory.size(); x++) {
            const auto y = shaken_solutions_trajectory[x];
            glVertex2d(x, y);
        }
        glEnd ();

        glColor4f(0.0f,1.0f,0.0f, 0.50f);
        glBegin (GL_POINTS);
        for(auto x = 0u; x < local_optima_trajectory.size(); x++) {
            const auto y = local_optima_trajectory[x];
            glVertex2f(x, y);
        }
        glEnd();

        glColor4f(1.0f,1.0f,0.0f, 1.0f);
        glBegin(GL_LINES);
        for(auto x = 0u; x < local_optima_trajectory.size()-1; x++) {
            const auto y = current_trajectory[x];
            glVertex2f(x, y);
            glVertex2d(x+1, current_trajectory[x+1]);
        }
        glEnd();

        glColor4f(0.0f,0.0f,1.0f, 1.0f);
        glBegin(GL_LINES);
        for(auto x = 0u; x < local_optima_trajectory.size()-1; x++) {
            const auto y = best_trajectory[x];
            glVertex2f(x, y);
            glVertex2d(x+1, best_trajectory[x+1]);
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();

    }


};

#endif //FILO__TRAJECTORY_RENDERER_HPP_