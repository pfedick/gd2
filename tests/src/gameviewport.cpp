#include <gtest/gtest.h>
#include "game.h"
#include "constants.h"

namespace
{

// The fixture for testing class GameViewport.
class GameViewportTest : public ::testing::Test
{
protected:
    GameViewport game_viewport;
};

TEST_F(GameViewportTest, ScalingAndGrid)
{
    // Test with 4K resolution
    game_viewport.setWindowSize({3840, 2160});
    const SDL_FRect& rect_4k = game_viewport.getRenderRect();
    EXPECT_EQ(rect_4k.x, 0);
    EXPECT_EQ(rect_4k.y, 0);
    EXPECT_EQ(rect_4k.w, 3840);
    EXPECT_EQ(rect_4k.h, 2160);
    EXPECT_FLOAT_EQ(game_viewport.getSpriteScaleFactor(), 1.0f);
    const ppl7::grafix::PointF& grid_4k = game_viewport.getGridSize();
    EXPECT_FLOAT_EQ(grid_4k.x, TILE_WIDTH * 2);
    EXPECT_FLOAT_EQ(grid_4k.y, TILE_HEIGHT * 2);

    // Test with Full HD
    game_viewport.setWindowSize({1920, 1080});
    const SDL_FRect& rect_hd = game_viewport.getRenderRect();
    EXPECT_EQ(rect_hd.x, 0);
    EXPECT_EQ(rect_hd.y, 0);
    EXPECT_EQ(rect_hd.w, 1920);
    EXPECT_EQ(rect_hd.h, 1080);
    EXPECT_FLOAT_EQ(game_viewport.getSpriteScaleFactor(), 0.5f);
    const ppl7::grafix::PointF& grid_hd = game_viewport.getGridSize();
    EXPECT_FLOAT_EQ(grid_hd.x, TILE_WIDTH);
    EXPECT_FLOAT_EQ(grid_hd.y, TILE_HEIGHT);

    // Test with an intermediate resolution
    game_viewport.setWindowSize({2560, 1440});
    const SDL_FRect& rect_qhd = game_viewport.getRenderRect();
    EXPECT_EQ(rect_qhd.x, 0);
    EXPECT_EQ(rect_qhd.y, 0);
    EXPECT_EQ(rect_qhd.w, 2560);
    EXPECT_EQ(rect_qhd.h, 1440);
    EXPECT_FLOAT_EQ(game_viewport.getSpriteScaleFactor(), 2560.0f / 3840.0f);
    const ppl7::grafix::PointF& grid_qhd = game_viewport.getGridSize();
    EXPECT_FLOAT_EQ(grid_qhd.x, (TILE_WIDTH * 2) * (2560.0f / 3840.0f));
    EXPECT_FLOAT_EQ(grid_qhd.y, (TILE_HEIGHT * 2) * (2560.0f / 3840.0f));
}

TEST_F(GameViewportTest, Letterboxing)
{
    // Test with a wider aspect ratio (e.g., 21:9) -> Pillarbox
    game_viewport.setWindowSize({2560, 1080});
    const SDL_FRect& rect_wide = game_viewport.getRenderRect();
    float expected_w = 1080.0f * (16.0f / 9.0f); // Height is the limit
    float expected_x = (2560.0f - expected_w) / 2.0f;
    EXPECT_FLOAT_EQ(rect_wide.w, expected_w);
    EXPECT_FLOAT_EQ(rect_wide.h, 1080);
    EXPECT_FLOAT_EQ(rect_wide.x, expected_x);
    EXPECT_FLOAT_EQ(rect_wide.y, 0);

    // Test with a taller aspect ratio (e.g., 4:3) -> Letterbox
    game_viewport.setWindowSize({1024, 768});
    const SDL_FRect& rect_tall = game_viewport.getRenderRect();
    float expected_h = 1024.0f / (16.0f / 9.0f); // Width is the limit
    float expected_y = (768.0f - expected_h) / 2.0f;
    EXPECT_FLOAT_EQ(rect_tall.w, 1024);
    EXPECT_FLOAT_EQ(rect_tall.h, expected_h);
    EXPECT_FLOAT_EQ(rect_tall.x, 0);
    EXPECT_FLOAT_EQ(rect_tall.y, expected_y);
}


TEST_F(GameViewportTest, CoordinateTranslation)
{
    // Use a standard 16:9 resolution
    game_viewport.setWindowSize({1920, 1080});

    // Translate a point
    ppl7::grafix::PointF translated_point = game_viewport.translate({100.0f, 100.0f});

    // The virtual resolution seems to be 3840x2160 (4K), based on the sprite scale calculation.
    // So a coordinate in a 1920x1080 window should be scaled by a factor of 2.
    // (3840 / 1920 = 2)
    float expected_x = 100.0f * (3840.0f / 1920.0f);
    float expected_y = 100.0f * (3840.0f / 1920.0f);

    EXPECT_FLOAT_EQ(translated_point.x, expected_x);
    EXPECT_FLOAT_EQ(translated_point.y, expected_y);
}

} // namespace
