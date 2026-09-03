#ifndef LEVELDATABASE_H
#define LEVELDATABASE_H

#include <array>

// 20 个关卡的初始棋盘数据（原 dataConfig 的紧凑版本）。
// 1 = 金币正面，0 = 银币反面；点击一个金币会翻转自身与上下相邻的金币。
namespace LevelDatabase {

inline constexpr int kRows = 4;
inline constexpr int kCols = 4;
inline constexpr int kCount = 20;

using Board = std::array<std::array<int, kCols>, kRows>;

// 返回全部关卡数据，有效下标为 1..kCount（下标 0 不使用）。
const std::array<Board, kCount + 1> &levels();

// 所有金币都为正面（1）时本关通关。
inline bool isSolved(const Board &board)
{
    for (const auto &row : board)
        for (int cell : row)
            if (cell != 1)
                return false;
    return true;
}

} // namespace LevelDatabase

#endif // LEVELDATABASE_H
