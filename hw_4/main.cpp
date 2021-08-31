#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    if (control_points.size() == 1) return control_points[0];
    std::vector<cv::Point2f> control_points_n;
    for (int i = 1; i < control_points.size(); i++) {
        cv::Point2f p_n = control_points[i - 1] + t * (control_points[i] - control_points[i - 1]);
        control_points_n.push_back(p_n);
    }
    return recursive_bezier(control_points_n, t);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    for (double t = 0.0; t <= 1.0; t += 0.001) {
        cv::Point2f point = recursive_bezier(control_points, t);

        // normal
        //window.at<cv::Vec3b>(point.y, point.x)[1] = 255;


        // anti aliasing
        cv::Point2f ld(int(point.x), int(point.y));
        cv::Point2f lu(int(point.x), int(point.y) + 1);
        cv::Point2f rd(int(point.x) + 1, int(point.y));
        cv::Point2f ru(int(point.x) + 1, int(point.y) + 1);

        float ld_dis = sqrt(pow(point.x - ld.x, 2.0) + pow(point.y - ld.y, 2.0));
        float lu_dis = sqrt(pow(point.x - lu.x, 2.0) + pow(point.y - lu.y, 2.0));
        float rd_dis = sqrt(pow(point.x - rd.x, 2.0) + pow(point.y - rd.y, 2.0));
        float ru_dis = sqrt(pow(point.x - ru.x, 2.0) + pow(point.y - ru.y, 2.0));

        window.at<cv::Vec3b>(ld.y, ld.x)[1] = std::max(
            window.at<cv::Vec3b>(ld.y, ld.x)[1], 
            uchar(std::max(1.0 - ld_dis, 0.0) * 255));
        window.at<cv::Vec3b>(lu.y, lu.x)[1] = std::max(
            window.at<cv::Vec3b>(lu.y, lu.x)[1], 
            uchar(std::max(1.0 - lu_dis, 0.0) * 255));
        window.at<cv::Vec3b>(rd.y, rd.x)[1] = std::max(
            window.at<cv::Vec3b>(rd.y, rd.x)[1], 
            uchar(std::max(1.0 - rd_dis, 0.0) * 255));
        window.at<cv::Vec3b>(ru.y, ru.x)[1] = std::max(
            window.at<cv::Vec3b>(ru.y, ru.x)[1], 
            uchar(std::max(1.0 - ru_dis, 0.0) * 255));
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            //naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
