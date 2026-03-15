#include "sbr2_pathfinder.h"

#include <algorithm>

SBR2PathFinder::SBR2PathFinder(
    const SBR2Board& board,
    const SBR2Simulator& simulator
)
    : board_(board),
      simulator_(simulator)
{
}

bool SBR2PathFinder::is_passable(i8 x, i8 y) const
{
    if (!board_.is_inside(x, y))
    {
        return false;
    }

    if (board_.is_hard_block(x, y))
    {
        return false;
    }

    if (board_.is_bomb(x, y))
    {
        return false;
    }

    return true;
}

bool SBR2PathFinder::is_safe(i8 x, i8 y, i32 frame) const
{
    if (!board_.is_inside(x, y))
    {
        return false;
    }

    return !simulator_.is_danger(frame, x, y);
}

bool SBR2PathFinder::is_safe_continuously(
    i8 x,
    i8 y,
    i32 from_frame,
    i32 to_frame
) const
{
    const i32 max_frames = SBR2Simulator::MAX_SIMULATION_FRAMES;

    if (from_frame < 0)
    {
        from_frame = 0;
    }

    if (to_frame < from_frame)
    {
        return true;
    }

    if (to_frame >= max_frames)
    {
        return false;
    }

    for (i32 f = from_frame; f <= to_frame; ++f)
    {
        if (!is_safe(x, y, f))
        {
            return false;
        }
    }

    return true;
}

bool SBR2PathFinder::can_wait(i8 x, i8 y, i32 frame) const
{
    if (!is_passable(x, y))
    {
        return false;
    }

    return is_safe(x, y, frame + 1);
}

bool SBR2PathFinder::can_move(i8 x, i8 y, i8 nx, i8 ny, i32 frame) const
{
    if (!board_.is_inside(nx, ny))
    {
        return false;
    }

    if (!is_passable(nx, ny))
    {
        return false;
    }

    // 移動中は元マスにいる扱い
    // frame+1 ～ frame+10 は元マスが安全である必要がある
    for (i32 f = frame + 1; f <= frame + MOVE_FRAMES - 1; ++f)
    {
        if (!is_safe(x, y, f))
        {
            return false;
        }
    }

    // frame+11 で次のマスに到達
    if (!is_safe(nx, ny, frame + MOVE_FRAMES))
    {
        return false;
    }

    return true;
}

bool SBR2PathFinder::can_reach_safe_cell(
    i8 start_x,
    i8 start_y,
    i32 start_frame
) const
{
    if (!is_passable(start_x, start_y))
    {
        return false;
    }

    if (!is_safe(start_x, start_y, start_frame))
    {
        return false;
    }

    const i32 max_frames = SBR2Simulator::MAX_SIMULATION_FRAMES;

    std::vector<std::vector<std::vector<bool>>> visited(
        max_frames,
        std::vector<std::vector<bool>>(
            SBR2Board::HEIGHT,
            std::vector<bool>(SBR2Board::WIDTH, false)
        )
    );

    std::queue<SBR2TimeNode> q;
    q.push({start_x, start_y, start_frame});
    visited[start_frame][start_y][start_x] = true;

    const i8 dx[4] = { 1, -1, 0, 0 };
    const i8 dy[4] = { 0, 0, 1, -1 };

    while (!q.empty())
    {
        SBR2TimeNode cur = q.front();
        q.pop();

        if (cur.frame + 2 < max_frames &&
            is_safe(cur.x, cur.y, cur.frame) &&
            is_safe(cur.x, cur.y, cur.frame + 1) &&
            is_safe(cur.x, cur.y, cur.frame + 2))
        {
            return true;
        }

        // WAIT
        {
            i32 next_frame = cur.frame + 1;

            if (next_frame < max_frames &&
                !visited[next_frame][cur.y][cur.x] &&
                can_wait(cur.x, cur.y, cur.frame))
            {
                visited[next_frame][cur.y][cur.x] = true;
                q.push({cur.x, cur.y, next_frame});
            }
        }

        // MOVE
        for (int dir = 0; dir < 4; ++dir)
        {
            i8 nx = cur.x + dx[dir];
            i8 ny = cur.y + dy[dir];
            i32 arrive_frame = cur.frame + MOVE_FRAMES;

            if (!board_.is_inside(nx, ny))
            {
                continue;
            }

            if (arrive_frame >= max_frames)
            {
                continue;
            }

            if (visited[arrive_frame][ny][nx])
            {
                continue;
            }

            if (!can_move(cur.x, cur.y, nx, ny, cur.frame))
            {
                continue;
            }

            visited[arrive_frame][ny][nx] = true;
            q.push({nx, ny, arrive_frame});
        }
    }

    return false;
}

bool SBR2PathFinder::find_safe_cell(
    i8 start_x,
    i8 start_y,
    i32 start_frame,
    i8& out_x,
    i8& out_y
) const
{
    const i32 max_frames = SBR2Simulator::MAX_SIMULATION_FRAMES;

    if (!board_.is_inside(start_x, start_y))
    {
        return false;
    }

    if (!is_passable(start_x, start_y))
    {
        return false;
    }

    if (!is_safe(start_x, start_y, start_frame))
    {
        return false;
    }

    if (start_frame < 0 || start_frame >= max_frames)
    {
        return false;
    }

    std::queue<SBR2TimeNode> q;
    std::vector<std::vector<std::vector<bool>>> visited(
        max_frames,
        std::vector<std::vector<bool>>(
            SBR2Board::HEIGHT,
            std::vector<bool>(SBR2Board::WIDTH, false)
        )
    );

    const i8 dx[4] = { 1, -1, 0, 0 };
    const i8 dy[4] = { 0, 0, 1, -1 };

    q.push({start_x, start_y, start_frame});
    visited[start_frame][start_y][start_x] = true;

    while (!q.empty())
    {
        SBR2TimeNode cur = q.front();
        q.pop();

        bool safe_long_enough = true;

        for (int t = 1; t <= 20; ++t)
        {
            int check_frame = cur.frame + t;

            if (check_frame >= max_frames)
            {
                break;
            }

            if (!is_safe(cur.x, cur.y, check_frame))
            {
                safe_long_enough = false;
                break;
            }
        }

        if (safe_long_enough)
        {
            out_x = cur.x;
            out_y = cur.y;
            return true;
        }

        // WAIT
        {
            i32 wait_frame = cur.frame + 1;

            if (wait_frame < max_frames &&
                !visited[wait_frame][cur.y][cur.x] &&
                can_wait(cur.x, cur.y, cur.frame))
            {
                visited[wait_frame][cur.y][cur.x] = true;
                q.push({cur.x, cur.y, wait_frame});
            }
        }

        // MOVE
        for (int dir = 0; dir < 4; ++dir)
        {
            i8 nx = cur.x + dx[dir];
            i8 ny = cur.y + dy[dir];
            i32 next_frame = cur.frame + MOVE_FRAMES;

            if (!board_.is_inside(nx, ny))
            {
                continue;
            }

            if (next_frame >= max_frames)
            {
                continue;
            }

            if (visited[next_frame][ny][nx])
            {
                continue;
            }

            if (!can_move(cur.x, cur.y, nx, ny, cur.frame))
            {
                continue;
            }

            visited[next_frame][ny][nx] = true;
            q.push({nx, ny, next_frame});
        }
    }

    return false;
}

bool SBR2PathFinder::find_escape_action(
    i8 start_x,
    i8 start_y,
    i32 start_frame,
    SBR2EscapeResult& out_result
) const
{
    const i32 safe_until_frame = start_frame + 20;
    return find_escape_action_until(
        start_x,
        start_y,
        start_frame,
        safe_until_frame,
        out_result
    );
}

bool SBR2PathFinder::find_escape_action_until(
    i8 start_x,
    i8 start_y,
    i32 start_frame,
    i32 safe_until_frame,
    SBR2EscapeResult& out_result
) const
{
    const i32 max_frames = SBR2Simulator::MAX_SIMULATION_FRAMES;

    if (!board_.is_inside(start_x, start_y))
    {
        return false;
    }

    if (!is_passable(start_x, start_y))
    {
        return false;
    }

    if (!is_safe(start_x, start_y, start_frame))
    {
        return false;
    }

    if (start_frame < 0 || start_frame >= max_frames)
    {
        return false;
    }

    if (safe_until_frame >= max_frames)
    {
        return false;
    }

    std::queue<int> q;
    std::vector<SBR2RouteNode> nodes;

    std::vector<std::vector<std::vector<bool>>> visited(
        max_frames,
        std::vector<std::vector<bool>>(
            SBR2Board::HEIGHT,
            std::vector<bool>(SBR2Board::WIDTH, false)
        )
    );

    const i8 dx[4] = { 1, -1, 0, 0 };
    const i8 dy[4] = { 0, 0, 1, -1 };
    const SBR2Action dir_action[4] = {
        SBR2Action::RIGHT,
        SBR2Action::LEFT,
        SBR2Action::DOWN,
        SBR2Action::UP
    };

    nodes.push_back({
        start_x,
        start_y,
        start_frame,
        -1,
        SBR2Action::WAIT
    });

    q.push(0);
    visited[start_frame][start_y][start_x] = true;

    while (!q.empty())
    {
        int cur_index = q.front();
        q.pop();

        const SBR2RouteNode& cur = nodes[cur_index];

        if (is_safe_continuously(cur.x, cur.y, cur.frame, safe_until_frame))
        {
            std::vector<SBR2Action> actions;
            int idx = cur_index;

            while (nodes[idx].parent_index != -1)
            {
                actions.push_back(nodes[idx].action_from_parent);
                idx = nodes[idx].parent_index;
            }

            std::reverse(actions.begin(), actions.end());

            out_result.found = true;
            out_result.target_x = cur.x;
            out_result.target_y = cur.y;
            out_result.first_action =
                actions.empty() ? SBR2Action::WAIT : actions[0];

            return true;
        }

        // WAIT
        {
            i32 next_frame = cur.frame + 1;

            if (next_frame < max_frames &&
                !visited[next_frame][cur.y][cur.x] &&
                can_wait(cur.x, cur.y, cur.frame))
            {
                visited[next_frame][cur.y][cur.x] = true;
                nodes.push_back({
                    cur.x,
                    cur.y,
                    next_frame,
                    cur_index,
                    SBR2Action::WAIT
                });
                q.push((int)nodes.size() - 1);
            }
        }

        // MOVE
        for (int dir = 0; dir < 4; ++dir)
        {
            i8 nx = cur.x + dx[dir];
            i8 ny = cur.y + dy[dir];
            i32 next_frame = cur.frame + MOVE_FRAMES;

            if (!board_.is_inside(nx, ny))
            {
                continue;
            }

            if (next_frame >= max_frames)
            {
                continue;
            }

            if (visited[next_frame][ny][nx])
            {
                continue;
            }

            if (!can_move(cur.x, cur.y, nx, ny, cur.frame))
            {
                continue;
            }

            visited[next_frame][ny][nx] = true;
            nodes.push_back({
                nx,
                ny,
                next_frame,
                cur_index,
                dir_action[dir]
            });
            q.push((int)nodes.size() - 1);
        }
    }

    return false;
}