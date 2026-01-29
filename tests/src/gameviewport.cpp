#include "game.h"
#include <gtest/gtest.h>

class GameViewportTest : public ::testing::Test
{
protected:
    GameViewport game_viewport;

    void SetUp() override
    {
        // Set a default aspect ratio for all tests
        game_viewport.setAspectRatio(16.0f / 9.0f);
    }
};

TEST_F(GameViewportTest, ScalingAndGrid)
{
    // Test case 1: Full HD resolution
    game_viewport.setWindowSize({1920, 1080});
    ASSERT_NEAR(game_viewport.getSpriteScaleFactor(), 0.5f, 0.001f);
    ppl7::grafix::PointF grid_size1 = game_viewport.getGridSize();
    ASSERT_NEAR(grid_size1.x, 32.0f, 0.001f);
    ASSERT_NEAR(grid_size1.y, 32.0f, 0.001f);

    // Test case 2: 4K resolution
    game_viewport.setWindowSize({3840, 2160});
    ASSERT_NEAR(game_viewport.getSpriteScaleFactor(), 1.0f, 0.001f);
    ppl7::grafix::PointF grid_size2 = game_viewport.getGridSize();
    ASSERT_NEAR(grid_size2.x, 64.0f, 0.001f);
    ASSERT_NEAR(grid_size2.y, 64.0f, 0.001f);

    // Test case 3: Resolution in between
    game_viewport.setWindowSize({2560, 1440});
    ASSERT_NEAR(game_viewport.getSpriteScaleFactor(), 2560.0f / 3840.0f, 0.001f);
    ppl7::grafix::PointF grid_size3 = game_viewport.getGridSize();
    ASSERT_NEAR(grid_size3.x, 32.0f * (2560.0f / 1920.0f), 0.001f);
    ASSERT_NEAR(grid_size3.y, 32.0f * (2560.0f / 1920.0f), 0.001f);
}

TEST_F(GameViewportTest, Letterboxing)
{
    // Test case 1: Wider aspect ratio (e.g., Ultrawide)
    game_viewport.setWindowSize({2560, 1080});
    const SDL_FRect& rect1 = game_viewport.getRenderRect();
    // Width should be scaled down to fit height, respecting aspect ratio
    float expected_width = 1080.0f * (16.0f / 9.0f);
    ASSERT_NEAR(rect1.w, expected_width, 0.001f);
    ASSERT_EQ(rect1.h, 1080);
    // Should be centered horizontally
    ASSERT_NEAR(rect1.x, (2560.0f - expected_width) / 2.0f, 0.001f);
    ASSERT_EQ(rect1.y, 0);

    // Test case 2: Taller aspect ratio (e.g., 4:3)
    game_viewport.setWindowSize({1440, 1080});
    const SDL_FRect& rect2 = game_viewport.getRenderRect();
    ASSERT_EQ(rect2.w, 1440);
    // Height should be scaled down to fit width, respecting aspect ratio
    float expected_height = 1440.0f / (16.0f / 9.0f);
    ASSERT_NEAR(rect2.h, expected_height, 0.001f);
    ASSERT_EQ(rect2.x, 0);
    // Should be centered vertically
    ASSERT_NEAR(rect2.y, (1080.0f - expected_height) / 2.0f, 0.001f);
}

TEST_F(GameViewportTest, CoordinateTranslation)
{
    // Case 1: Standard 16:9 resolution - No letterboxing
    game_viewport.setWindowSize({1920, 1080});
    ppl7::grafix::PointF point1 = {100.0f, 200.0f};
    ppl7::grafix::PointF translated1 = game_viewport.translate(point1);

    // With no letterboxing, coordinates should be identical.
    EXPECT_FLOAT_EQ(translated1.x, 100.0f);
    EXPECT_FLOAT_EQ(translated1.y, 200.0f);

    // Case 2: Wider window - Horizontal letterboxing (bars on the sides)
    game_viewport.setWindowSize({2560, 1080}); // e.g., an ultrawide monitor
    const SDL_FRect& render_rect2 = game_viewport.getRenderRect(); // h=1080, w=1920, x=320, y=0

    ppl7::grafix::PointF point2 = {420.0f, 300.0f}; // A point inside the render area
    ppl7::grafix::PointF translated2 = game_viewport.translate(point2);

    // The translation must subtract the x-offset of the letterbox.
    // expected_x = input_x - render_rect.x = 420 - 320 = 100
    EXPECT_FLOAT_EQ(translated2.x, 100.0f);
    EXPECT_FLOAT_EQ(translated2.y, 300.0f); // Y has no offset

    // Case 3: Taller window - Vertical letterboxing (bars top and bottom)
    game_viewport.setWindowSize({1920, 1440});
    const SDL_FRect& render_rect3 = game_viewport.getRenderRect(); // h=1080, w=1920, x=0, y=180

    ppl7::grafix::PointF point3 = {500.0f, 280.0f}; // A point inside the render area
    ppl7::grafix::PointF translated3 = game_viewport.translate(point3);

    // The translation must subtract the y-offset of the letterbox.
    // expected_y = input_y - render_rect.y = 280 - 180 = 100
    EXPECT_FLOAT_EQ(translated3.x, 500.0f); // X has no offset
    EXPECT_FLOAT_EQ(translated3.y, 100.0f);
}