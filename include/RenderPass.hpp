#pragma once


class RenderAttachment
{

};

class ColorAttachment : RenderAttachment
{

};

class DepthAttachment : RenderAttachment
{

};





class SubPass
{

};

class GraphicsSubPass : SubPass
{

};

class RaytracingSubPass : SubPass
{

}




template <typename... SubPasses>
class RenderPass
{

};