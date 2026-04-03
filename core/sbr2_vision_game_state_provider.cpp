#include "core/sbr2_vision_game_state_provider.h"

#include <iostream>
#include <utility>
#include "core/sbr2_screen_capture.h"
#include <algorithm>

namespace
{
    constexpr bool kUseDummyVisionPhase = true;
    constexpr bool kUseDummyVisionSelfPosition = true;
    constexpr bool kUseDummyVisionEnemyPosition = true;

    constexpr int kReadyFrames = 50;
    constexpr int kGoFrames = 100;
    constexpr int kResultFrames = 30;

    constexpr int kGoStartFrame = kReadyFrames;
    constexpr int kResultStartFrame = kGoStartFrame + kGoFrames;
    constexpr int kRoundFrames = kResultStartFrame + kResultFrames;
    constexpr int kRoundCount = 4;

    bool is_ready_visible(int tick)
    {
        if (!kUseDummyVisionPhase)
        {
            return false;
        }

        int round_tick = tick % kRoundFrames;
        return round_tick < kReadyFrames;
    }

    bool is_result_visible(int tick)
    {
        if (!kUseDummyVisionPhase)
        {
            return false;
        }

        int round_tick = tick % kRoundFrames;
        return round_tick >= kResultStartFrame;
    }

    SBR2Phase detect_phase(int tick)
    {
        (void)tick;
        return SBR2Phase::GO;
    }

    std::pair<int, int> detect_self_position_from_frame(int tick)
    {
        if (!kUseDummyVisionSelfPosition)
        {
            return {-1, -1};
        }

        int round_index = (tick / kRoundFrames) % kRoundCount;

        switch (round_index)
        {
        case 0:
            return {0, 0};
        case 1:
            return {12, 0};
        case 2:
            return {0, 10};
        default:
            return {12, 10};
        }
    }

    std::pair<int, int> detect_enemy_position_from_frame(int tick)
    {
        if (!kUseDummyVisionEnemyPosition)
        {
            return {-1, -1};
        }

        (void)tick;
        return {6, 5};
    }

    std::pair<int, int> detect_self_position(int tick)
    {
        return detect_self_position_from_frame(tick);
    }

    std::pair<int, int> detect_enemy_position(int tick)
    {
        return detect_enemy_position_from_frame(tick);
    }

    struct SpawnCornerProbe
    {
        int board_x = -1;
        int board_y = -1;
        double x0_ratio = 0.0;
        double y0_ratio = 0.0;
        double x1_ratio = 0.0;
        double y1_ratio = 0.0;
    };

    int count_opening_player_like_pixels(const SBR2Image &image,
                                         double x0_ratio,
                                         double y0_ratio,
                                         double x1_ratio,
                                         double y1_ratio)
    {
        if (image.width <= 0 || image.height <= 0)
        {
            return 0;
        }

        int x0 = std::clamp(static_cast<int>(image.width * x0_ratio), 0, image.width);
        int y0 = std::clamp(static_cast<int>(image.height * y0_ratio), 0, image.height);
        int x1 = std::clamp(static_cast<int>(image.width * x1_ratio), 0, image.width);
        int y1 = std::clamp(static_cast<int>(image.height * y1_ratio), 0, image.height);

        if (x0 >= x1 || y0 >= y1)
        {
            return 0;
        }

        int count = 0;

        for (int y = y0; y < y1; ++y)
        {
            for (int x = x0; x < x1; ++x)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                     static_cast<std::size_t>(x)) *
                    4;

                const std::uint8_t b = image.pixels[index + 0];
                const std::uint8_t g = image.pixels[index + 1];
                const std::uint8_t r = image.pixels[index + 2];

                const int max_rg = (r > g) ? r : g;
                const int maxc = (max_rg > b) ? max_rg : b;

                const int min_rg = (r < g) ? r : g;
                const int minc = (min_rg < b) ? min_rg : b;

                const int sat = maxc - minc;
                const int lum = (static_cast<int>(r) + static_cast<int>(g) + static_cast<int>(b)) / 3;

                const bool too_white = (lum >= 225 && sat <= 18);
                const bool too_dark = (lum <= 16);
                const bool green_floor_like = (g >= 70 && g >= r + 12 && g >= b + 12);
                const bool blue_floor_like = (b >= 70 && b >= r + 12 && b >= g + 8);
                const bool warm_ui_like = (r >= 190 && g >= 135 && b <= 120);

                const bool player_like =
                    !too_white &&
                    !too_dark &&
                    !green_floor_like &&
                    !blue_floor_like &&
                    !warm_ui_like &&
                    (sat >= 28 || lum <= 95);

                if (player_like)
                {
                    ++count;
                }
            }
        }

        return count;
    }

    std::pair<int, int> detect_opening_corner_by_rank_from_image(const SBR2Image &image, int rank)
    {
        static const SpawnCornerProbe probes[4] = {
            {0, 0, 0.02, 0.06, 0.18, 0.26},
            {12, 0, 0.82, 0.06, 0.98, 0.26},
            {0, 10, 0.02, 0.74, 0.18, 0.94},
            {12, 10, 0.82, 0.74, 0.98, 0.94},
        };

        struct ScoredCorner
        {
            int board_x = -1;
            int board_y = -1;
            int score = 0;
        };

        ScoredCorner scored[4];

        for (int i = 0; i < 4; ++i)
        {
            scored[i].board_x = probes[i].board_x;
            scored[i].board_y = probes[i].board_y;
            scored[i].score = count_opening_player_like_pixels(
                image,
                probes[i].x0_ratio,
                probes[i].y0_ratio,
                probes[i].x1_ratio,
                probes[i].y1_ratio);
        }

        for (int i = 0; i < 4; ++i)
        {
            for (int j = i + 1; j < 4; ++j)
            {
                if (scored[j].score > scored[i].score)
                {
                    const ScoredCorner tmp = scored[i];
                    scored[i] = scored[j];
                    scored[j] = tmp;
                }
            }
        }

        if (rank < 0 || rank >= 4)
        {
            return {-1, -1};
        }

        const int kMinCornerScore = 120;
        if (scored[rank].score < kMinCornerScore)
        {
            return {-1, -1};
        }

        return {scored[rank].board_x, scored[rank].board_y};
    }

    struct SBR2RectI
    {
        int x0 = 0;
        int y0 = 0;
        int x1 = 0;
        int y1 = 0;
    };

    SBR2RectI make_rect_from_ratio(const SBR2Image &image,
                                   double x0_ratio,
                                   double y0_ratio,
                                   double x1_ratio,
                                   double y1_ratio)
    {
        SBR2RectI r;
        r.x0 = std::clamp(static_cast<int>(image.width * x0_ratio), 0, image.width);
        r.y0 = std::clamp(static_cast<int>(image.height * y0_ratio), 0, image.height);
        r.x1 = std::clamp(static_cast<int>(image.width * x1_ratio), 0, image.width);
        r.y1 = std::clamp(static_cast<int>(image.height * y1_ratio), 0, image.height);
        return r;
    }

    bool is_bright_ready_like_pixel(std::uint8_t r,
                                    std::uint8_t g,
                                    std::uint8_t b)
    {
        const bool white_like = (r >= 210 && g >= 210 && b >= 210);
        const bool warm_like = (r >= 180 && g >= 140 && b <= 170);
        return white_like || warm_like;
    }

    int count_ready_like_pixels(const SBR2Image &image, const SBR2RectI &rect)
    {
        if (image.width <= 0 || image.height <= 0)
        {
            return 0;
        }

        if (rect.x0 >= rect.x1 || rect.y0 >= rect.y1)
        {
            return 0;
        }

        int count = 0;

        for (int y = rect.y0; y < rect.y1; ++y)
        {
            for (int x = rect.x0; x < rect.x1; ++x)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                     static_cast<std::size_t>(x)) *
                    4;

                const std::uint8_t b = image.pixels[index + 0];
                const std::uint8_t g = image.pixels[index + 1];
                const std::uint8_t r = image.pixels[index + 2];

                if (is_bright_ready_like_pixel(r, g, b))
                {
                    ++count;
                }
            }
        }

        return count;
    }

    bool detect_ready_from_image(const SBR2Image &image,
                                 int &out_total_count,
                                 int &out_left_count,
                                 int &out_right_count)
    {
        SBR2RectI ready_total_rect = make_rect_from_ratio(image, 0.33, 0.47, 0.67, 0.63);
        SBR2RectI ready_left_rect = make_rect_from_ratio(image, 0.33, 0.47, 0.48, 0.63);
        SBR2RectI ready_right_rect = make_rect_from_ratio(image, 0.52, 0.47, 0.67, 0.63);

        out_total_count = count_ready_like_pixels(image, ready_total_rect);
        out_left_count = count_ready_like_pixels(image, ready_left_rect);
        out_right_count = count_ready_like_pixels(image, ready_right_rect);

        const bool total_ok = (out_total_count >= 2000);
        const bool left_ok = (out_left_count >= 600);
        const bool right_ok = (out_right_count >= 600);

        return total_ok && left_ok && right_ok;
    }

    bool is_go_like_pixel(std::uint8_t r,
                          std::uint8_t g,
                          std::uint8_t b)
    {
        const bool green_like =
            (g >= 170) &&
            (r >= 120) &&
            (b <= 170) &&
            (g >= r) &&
            (g >= b);

        const bool bright_white_like =
            (r >= 220 && g >= 220 && b >= 220);

        return green_like || bright_white_like;
    }

    int count_go_like_pixels(const SBR2Image &image, const SBR2RectI &rect)
    {
        if (image.width <= 0 || image.height <= 0)
        {
            return 0;
        }

        if (rect.x0 >= rect.x1 || rect.y0 >= rect.y1)
        {
            return 0;
        }

        int count = 0;

        for (int y = rect.y0; y < rect.y1; ++y)
        {
            for (int x = rect.x0; x < rect.x1; ++x)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                     static_cast<std::size_t>(x)) *
                    4;

                const std::uint8_t b = image.pixels[index + 0];
                const std::uint8_t g = image.pixels[index + 1];
                const std::uint8_t r = image.pixels[index + 2];

                if (is_go_like_pixel(r, g, b))
                {
                    ++count;
                }
            }
        }

        return count;
    }

    bool detect_go_from_image(const SBR2Image &image,
                              int &out_total_count,
                              int &out_left_count,
                              int &out_right_count)
    {
        SBR2RectI go_total_rect = make_rect_from_ratio(image, 0.40, 0.47, 0.60, 0.63);
        SBR2RectI go_left_rect = make_rect_from_ratio(image, 0.40, 0.47, 0.49, 0.63);
        SBR2RectI go_right_rect = make_rect_from_ratio(image, 0.51, 0.47, 0.60, 0.63);

        out_total_count = count_go_like_pixels(image, go_total_rect);
        out_left_count = count_go_like_pixels(image, go_left_rect);
        out_right_count = count_go_like_pixels(image, go_right_rect);

        const bool total_ok = (out_total_count >= 600);
        const bool left_ok = (out_left_count >= 250);
        const bool right_ok = (out_right_count >= 250);

        return total_ok && left_ok && right_ok;
    }

    bool is_winner_like_pixel(std::uint8_t r,
                              std::uint8_t g,
                              std::uint8_t b)
    {
        const bool gold_like =
            (r >= 180) &&
            (g >= 140) &&
            (b <= 140) &&
            (r >= g) &&
            (g >= b);

        const bool bright_white_like =
            (r >= 205) &&
            (g >= 205) &&
            (b >= 205);

        const bool silver_white_like =
            (r >= 170) &&
            (g >= 170) &&
            (b >= 170) &&
            (std::abs(static_cast<int>(r) - static_cast<int>(g)) <= 25) &&
            (std::abs(static_cast<int>(g) - static_cast<int>(b)) <= 25);

        return gold_like || bright_white_like || silver_white_like;
    }

    int count_winner_like_pixels(const SBR2Image &image, const SBR2RectI &rect)
    {
        if (image.width <= 0 || image.height <= 0)
        {
            return 0;
        }

        if (rect.x0 >= rect.x1 || rect.y0 >= rect.y1)
        {
            return 0;
        }

        int count = 0;

        for (int y = rect.y0; y < rect.y1; ++y)
        {
            for (int x = rect.x0; x < rect.x1; ++x)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                     static_cast<std::size_t>(x)) *
                    4;

                const std::uint8_t b = image.pixels[index + 0];
                const std::uint8_t g = image.pixels[index + 1];
                const std::uint8_t r = image.pixels[index + 2];

                if (is_winner_like_pixel(r, g, b))
                {
                    ++count;
                }
            }
        }

        return count;
    }

    bool detect_winner_from_image(const SBR2Image &image,
                                  int &out_total_count,
                                  int &out_left_count,
                                  int &out_right_count)
    {
        SBR2RectI winner_total_rect = make_rect_from_ratio(image, 0.30, 0.44, 0.70, 0.62);
        SBR2RectI winner_left_rect = make_rect_from_ratio(image, 0.30, 0.44, 0.49, 0.62);
        SBR2RectI winner_right_rect = make_rect_from_ratio(image, 0.51, 0.44, 0.70, 0.62);

        out_total_count = count_winner_like_pixels(image, winner_total_rect);
        out_left_count = count_winner_like_pixels(image, winner_left_rect);
        out_right_count = count_winner_like_pixels(image, winner_right_rect);

        const bool total_ok = (out_total_count >= 1400);
        const bool left_ok = (out_left_count >= 420);
        const bool right_ok = (out_right_count >= 420);

        return total_ok && left_ok && right_ok;
    }

    bool is_draw_like_pixel(std::uint8_t r,
                            std::uint8_t g,
                            std::uint8_t b)
    {
        const bool bright_white_like =
            (r >= 205) &&
            (g >= 205) &&
            (b >= 205);

        const bool silver_white_like =
            (r >= 170) &&
            (g >= 170) &&
            (b >= 170) &&
            (std::abs(static_cast<int>(r) - static_cast<int>(g)) <= 25) &&
            (std::abs(static_cast<int>(g) - static_cast<int>(b)) <= 25);

        return bright_white_like || silver_white_like;
    }

    int count_draw_like_pixels(const SBR2Image &image, const SBR2RectI &rect)
    {
        if (image.width <= 0 || image.height <= 0)
        {
            return 0;
        }

        if (rect.x0 >= rect.x1 || rect.y0 >= rect.y1)
        {
            return 0;
        }

        int count = 0;

        for (int y = rect.y0; y < rect.y1; ++y)
        {
            for (int x = rect.x0; x < rect.x1; ++x)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                     static_cast<std::size_t>(x)) *
                    4;

                const std::uint8_t b = image.pixels[index + 0];
                const std::uint8_t g = image.pixels[index + 1];
                const std::uint8_t r = image.pixels[index + 2];

                if (is_draw_like_pixel(r, g, b))
                {
                    ++count;
                }
            }
        }

        return count;
    }

    bool detect_draw_from_image(const SBR2Image &image,
                                int &out_total_count,
                                int &out_left_count,
                                int &out_right_count)
    {
        SBR2RectI draw_total_rect = make_rect_from_ratio(image, 0.31, 0.43, 0.69, 0.61);
        SBR2RectI draw_left_rect = make_rect_from_ratio(image, 0.31, 0.43, 0.49, 0.61);
        SBR2RectI draw_right_rect = make_rect_from_ratio(image, 0.51, 0.43, 0.69, 0.61);

        out_total_count = count_draw_like_pixels(image, draw_total_rect);
        out_left_count = count_draw_like_pixels(image, draw_left_rect);
        out_right_count = count_draw_like_pixels(image, draw_right_rect);

        const bool total_ok = (out_total_count >= 900);
        const bool left_ok = (out_left_count >= 250);
        const bool right_ok = (out_right_count >= 250);

        return total_ok && left_ok && right_ok;
    }

    bool is_result_band_like_pixel(std::uint8_t r,
                                   std::uint8_t g,
                                   std::uint8_t b)
    {
        const bool orange_like =
            (r >= 220) &&
            (g >= 120) &&
            (g <= 210) &&
            (b <= 120) &&
            (r > g);

        const bool yellow_like =
            (r >= 220) &&
            (g >= 200) &&
            (b <= 130);

        return orange_like || yellow_like;
    }

    int count_result_band_like_pixels(const SBR2Image &image, const SBR2RectI &rect)
    {
        if (image.width <= 0 || image.height <= 0)
        {
            return 0;
        }

        if (rect.x0 >= rect.x1 || rect.y0 >= rect.y1)
        {
            return 0;
        }

        int count = 0;

        for (int y = rect.y0; y < rect.y1; ++y)
        {
            for (int x = rect.x0; x < rect.x1; ++x)
            {
                const std::size_t index =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                     static_cast<std::size_t>(x)) *
                    4;

                const std::uint8_t b = image.pixels[index + 0];
                const std::uint8_t g = image.pixels[index + 1];
                const std::uint8_t r = image.pixels[index + 2];

                if (is_result_band_like_pixel(r, g, b))
                {
                    ++count;
                }
            }
        }

        return count;
    }
}

SBR2GameState SBR2VisionGameStateProvider::get_state(int tick)
{
    if (tick % 30 == 0)
    {
        std::cout << "[vision] get_state tick=" << tick << std::endl;
    }

    SBR2GameState s;
    SBR2Image image;
    capture_screen(image);

    static int ready_visible_streak = 0;
    static int ready_hidden_streak = 0;
    static bool ready_active = false;

    static int go_visible_streak = 0;
    static int go_hidden_streak = 0;
    static bool go_active = false;

    static int winner_visible_streak = 0;
    static int draw_visible_streak = 0;

    static bool go_open_delay_started = false;
    static int go_open_delay_frames_left = 0;

    static int result_visible_streak = 0;
    static int result_hidden_streak = 0;
    static bool result_active = false;

    SBR2RectI ready_total_rect = make_rect_from_ratio(image, 0.28, 0.42, 0.72, 0.72);
    SBR2RectI ready_left_rect = make_rect_from_ratio(image, 0.28, 0.42, 0.42, 0.72);
    SBR2RectI ready_right_rect = make_rect_from_ratio(image, 0.58, 0.42, 0.72, 0.72);

    int ready_total_count = 0;
    int ready_left_count = 0;
    int ready_right_count = 0;

    bool ready_visible = detect_ready_from_image(
        image,
        ready_total_count,
        ready_left_count,
        ready_right_count);

    if (ready_visible)
    {
        ++ready_visible_streak;
        ready_hidden_streak = 0;
    }
    else
    {
        ready_visible_streak = 0;
        ++ready_hidden_streak;
    }

    if (!ready_active && ready_visible_streak >= 3)
    {
        ready_active = true;
    }

    if (ready_active && ready_hidden_streak >= 2)
    {
        ready_active = false;
    }

    bool ready_confirmed = ready_active;

    if (tick % 30 == 0)
    {
        std::cout
            << "[vision][ready_probe] visible=" << (ready_visible ? 1 : 0)
            << " active=" << (ready_active ? 1 : 0)
            << " v_streak=" << ready_visible_streak
            << " h_streak=" << ready_hidden_streak
            << " total=" << ready_total_count
            << " left=" << ready_left_count
            << " right=" << ready_right_count
            << std::endl;
    }

    int go_total_count = 0;
    int go_left_count = 0;
    int go_right_count = 0;

    bool go_visible = detect_go_from_image(
        image,
        go_total_count,
        go_left_count,
        go_right_count);

    if (go_visible)
    {
        ++go_visible_streak;
        go_hidden_streak = 0;
    }
    else
    {
        go_visible_streak = 0;
        ++go_hidden_streak;
    }

    if (!go_active && go_visible_streak >= 2)
    {
        go_active = true;
    }

    if (go_active && go_hidden_streak >= 2)
    {
        go_active = false;
    }

    bool go_confirmed = go_active;
    if (ready_confirmed)
    {
        go_open_delay_started = false;
        go_open_delay_frames_left = 0;
    }

    if (!go_open_delay_started && go_visible)
    {
        go_open_delay_started = true;
        go_open_delay_frames_left = 2;
    }

    bool go_open_delay_active =
        go_open_delay_started && (go_open_delay_frames_left > 0);

    if (go_open_delay_frames_left > 0)
    {
        --go_open_delay_frames_left;
    }

    if (tick % 30 == 0)
    {
        std::cout
            << "[vision][go_probe] visible=" << (go_visible ? 1 : 0)
            << " confirmed=" << (go_confirmed ? 1 : 0)
            << " streak=" << go_visible_streak
            << " open_delay_started=" << (go_open_delay_started ? 1 : 0)
            << " open_delay_left=" << go_open_delay_frames_left
            << " open_delay_active=" << (go_open_delay_active ? 1 : 0)
            << " total=" << go_total_count
            << " left=" << go_left_count
            << " right=" << go_right_count
            << std::endl;
    }

    int winner_total_count = 0;
    int winner_left_count = 0;
    int winner_right_count = 0;

    bool winner_visible = detect_winner_from_image(
        image,
        winner_total_count,
        winner_left_count,
        winner_right_count);

    SBR2RectI result_band_rect = make_rect_from_ratio(image, 0.28, 0.40, 0.72, 0.64);
    int result_band_count = count_result_band_like_pixels(image, result_band_rect);
    bool result_band_active = (result_band_count >= 3500);

    if (result_band_active)
    {
        winner_visible = false;
    }

    bool winner_right_noise =
        (winner_right_count >= 2200) &&
        (winner_left_count <= 300);

    if (winner_right_noise)
    {
        winner_visible = false;
    }

    if (winner_visible)
    {
        ++winner_visible_streak;
    }
    else
    {
        winner_visible_streak = 0;
    }

    bool winner_confirmed = (winner_visible_streak >= 8);

    if ((tick % 5 == 0) || winner_visible || (winner_total_count >= 600))
    {
        std::cout
            << "[vision][winner_probe] visible=" << (winner_visible ? 1 : 0)
            << " confirmed=" << (winner_confirmed ? 1 : 0)
            << " streak=" << winner_visible_streak
            << " total=" << winner_total_count
            << " left=" << winner_left_count
            << " right=" << winner_right_count
            << std::endl;
    }

    int draw_total_count = 0;
    int draw_left_count = 0;
    int draw_right_count = 0;

    bool draw_visible = detect_draw_from_image(
        image,
        draw_total_count,
        draw_left_count,
        draw_right_count);

    if (result_band_active)
    {
        draw_visible = false;
    }

    if (draw_visible)
    {
        ++draw_visible_streak;
    }
    else
    {
        draw_visible_streak = 0;
    }

    bool draw_confirmed = (draw_visible_streak >= 2);

    bool result_visible_raw = (winner_visible || draw_visible);
    bool result_allowed = (!ready_active && !go_active);
    bool result_visible = result_allowed && result_visible_raw;

    if (result_visible)
    {
        ++result_visible_streak;
        result_hidden_streak = 0;
    }
    else
    {
        result_visible_streak = 0;
        ++result_hidden_streak;
    }

    if (!result_active && result_visible_streak >= 4)
    {
        result_active = true;
    }

    if (result_active && result_hidden_streak >= 1)
    {
        result_active = false;
    }

    bool result_confirmed = result_active;

    if (ready_active || go_active)
    {
        winner_visible_streak = 0;
        draw_visible_streak = 0;
        result_visible_streak = 0;
        result_hidden_streak = 0;
        result_active = false;
        result_confirmed = false;
    }

    if (tick % 30 == 0)
    {
        std::cout
            << "[vision][draw_probe] visible=" << (draw_visible ? 1 : 0)
            << " confirmed=" << (draw_confirmed ? 1 : 0)
            << " streak=" << draw_visible_streak
            << " total=" << draw_total_count
            << " left=" << draw_left_count
            << " right=" << draw_right_count
            << std::endl;
    }

    if (tick % 30 == 0)
    {
        std::cout
            << "[vision][result_gate] allowed=" << (result_allowed ? 1 : 0)
            << " raw=" << (result_visible_raw ? 1 : 0)
            << " active=" << (result_active ? 1 : 0)
            << " v_streak=" << result_visible_streak
            << " h_streak=" << result_hidden_streak
            << std::endl;
    }

    if (tick % 30 == 0)
    {
        std::cout
            << "[vision][result_band] active=" << (result_band_active ? 1 : 0)
            << " count=" << result_band_count
            << std::endl;
    }

    if ((ready_active || go_active) && !result_band_active && tick % 30 == 0)
    {
        const auto corner0 = detect_opening_corner_by_rank_from_image(image, 0);
        const auto corner1 = detect_opening_corner_by_rank_from_image(image, 1);

        std::cout
            << "[vision][opening_corners]"
            << " c0=(" << corner0.first << "," << corner0.second << ")"
            << " c1=(" << corner1.first << "," << corner1.second << ")"
            << std::endl;
    }

    auto self_pos = detect_self_position(tick);
    auto enemy_pos = detect_enemy_position(tick);

    s.self_found = (self_pos.first >= 0 && self_pos.second >= 0);
    s.self_x = self_pos.first;
    s.self_y = self_pos.second;

    s.enemy_found = (enemy_pos.first >= 0 && enemy_pos.second >= 0);
    s.enemy_x = enemy_pos.first;
    s.enemy_y = enemy_pos.second;

    SBR2Phase phase = detect_phase(tick);

    if (result_active)
    {
        phase = SBR2Phase::RESULT;
    }
    else if (ready_active)
    {
        // 本物の READY は優先して通す。
        // ラウンド遷移帯が少し重なっても、READY を見たら ai_pad_test 側の
        // controls_unlocked_for_round を解除できるようにする。
        phase = SBR2Phase::READY;
    }
    else if (result_band_active)
    {
        // READY が見えていない帯演出だけ UNKNOWN にする。
        phase = SBR2Phase::UNKNOWN;
    }
    else if (go_active)
    {
        phase = SBR2Phase::GO;
    }

    s.phase = phase;
    s.go_open_delay_active = go_open_delay_active;

    return s;
}