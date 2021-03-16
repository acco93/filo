//
// Created by acco on 2/3/20.
//

#ifndef FILO__SOLUTION_RENDERER_HPP_
#define FILO__SOLUTION_RENDERER_HPP_

#include <GLFW/glfw3.h>
#include <cobra/Instance.hpp>
#include <cobra/Solution.hpp>
#include <cobra/MoveGenerators.hpp>

class SolutionRenderer {

    GLFWwindow* window = nullptr;

    float x_min{}, x_max{}, y_min{}, y_max{};

 public:

    SolutionRenderer(const cobra::Instance& instance) {

        if(!glfwInit()) {
            std::cerr << "Cannot successfully execute 'glfwInit'\n";
            abort();
        }

        window = glfwCreateWindow(960, 960, "FILO", nullptr, nullptr);

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

        x_min = instance.get_x_coordinate(instance.get_vertices_begin());
        x_max = x_min;
        y_min = instance.get_y_coordinate(instance.get_vertices_begin());
        y_max = y_min;
        for(auto i = instance.get_vertices_begin(); i < instance.get_vertices_end(); i++) {
            const auto x_i = instance.get_x_coordinate(i);
            const auto y_i = instance.get_y_coordinate(i);
            x_min = std::min(x_min, x_i);
            x_max = std::max(x_max, x_i);
            y_min = std::min(y_min, y_i);
            y_max = std::max(y_max, y_i);
        }


    }

    void draw(const cobra::Instance& instance, cobra::Solution& solution,
              const cobra::LRUCache& cached_vertices,
              cobra::MoveGenerators& move_generators) {

        glfwMakeContextCurrent(window);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(x_min, x_max, y_min, y_max, -1, 1);

        glMatrixMode(GL_MODELVIEW);

        glClearColor(0.20, 0.20, 0.20, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        for(auto i = instance.get_vertices_begin(); i < instance.get_vertices_end(); i++) {

            for(auto move_id : move_generators.get_move_generator_indices_involving(i)) {

                const auto& move = move_generators.get(move_id);

                glBegin(GL_LINES);
                glColor4f(0.0, 1.0 ,1.0, 0.10);
                glVertex2d(instance.get_x_coordinate(move.get_first_vertex()), instance.get_y_coordinate(move.get_first_vertex()));
                glVertex2d(instance.get_x_coordinate(move.get_second_vertex()), instance.get_y_coordinate(move.get_second_vertex()));
                glEnd();

            }

        }



        for(auto i = cached_vertices.begin(); i != cobra::LRUCache::Entry::dummy_vertex; i = cached_vertices.get_next(i)) {

            //glBegin (GL_POINTS);
            //glPointSize(1);
            //glColor4f(1, 1, 0,0.5);
            //glVertex2d(instance.get_x_coordinate(i), instance.get_y_coordinate(i));
            //glEnd();

            for(auto move_id : move_generators.get_move_generator_indices_involving(i)) {
                const auto &move = move_generators.get(move_id);
                glBegin(GL_LINES);
                glColor4f(1.0, 1.0, 0.0, 0.50);
                glVertex2d(instance.get_x_coordinate(move.get_first_vertex()),instance.get_y_coordinate(move.get_first_vertex()));
                glVertex2d(instance.get_x_coordinate(move.get_second_vertex()), instance.get_y_coordinate(move.get_second_vertex()));
                glEnd();
            }
        }


        glLineWidth(2);
        for(auto route = solution.get_first_route(); route != cobra::Solution::dummy_route; route = solution.get_next_route(route)) {

            const auto load_ratio = static_cast<float>(solution.get_route_load(route)) / static_cast<float>(instance.get_vehicle_capacity());


            glPushAttrib(GL_ENABLE_BIT);
            glLineStipple(1, 0xAAAA);
            glEnable(GL_LINE_STIPPLE);
            glColor4f(load_ratio,1-load_ratio,0.00, 0.1f);
            glBegin(GL_LINES);
            glVertex2d(instance.get_x_coordinate(instance.get_depot()), instance.get_y_coordinate(instance.get_depot()));
            glVertex2d(instance.get_x_coordinate(solution.get_first_customer(route)), instance.get_y_coordinate(solution.get_first_customer(route)));
            glEnd();
            glPopAttrib();


            glBegin(GL_LINES);

            glColor4f(load_ratio,1-load_ratio,0.00, 1.0f);

            auto curr = solution.get_first_customer(route);
            auto next = curr;

            do {

                next = solution.get_next_vertex(route, curr);

                if(next == instance.get_depot()) { break; }

                glVertex2f(instance.get_x_coordinate(curr), instance.get_y_coordinate(curr));
                glVertex2f(instance.get_x_coordinate(next), instance.get_y_coordinate(next));

                curr = next;


            } while(true);



            glEnd();

            glPushAttrib(GL_ENABLE_BIT);
            glLineStipple(1, 0xAAAA);
            glEnable(GL_LINE_STIPPLE);
            glColor4f(load_ratio,1-load_ratio,0.0, 0.1f);
            glBegin(GL_LINES);
            glVertex2d(instance.get_x_coordinate(solution.get_last_customer(route)), instance.get_y_coordinate(solution.get_last_customer(route)));
            glVertex2d(instance.get_x_coordinate(instance.get_depot()), instance.get_y_coordinate(instance.get_depot()));
            glEnd();
            glPopAttrib();

        }

        glPointSize(10);
        glBegin (GL_POINTS);
        glColor4f(1.0,1.0,1.0,1.0);
        glVertex2d(instance.get_x_coordinate(instance.get_depot()), instance.get_y_coordinate(instance.get_depot()));
        glEnd ();

        glfwSwapBuffers(window);

        glfwPollEvents();

    }


};


#endif //FILO__SOLUTION_RENDERER_HPP_