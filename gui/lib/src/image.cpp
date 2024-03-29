#include "image.h"
namespace Vulkan
{

Image::Image(Command* commander,int window_x,int window_y,int x_size,int y_size,int x_pos,int y_pos):device_(commander->device_),commander_(commander),x_size_(x_size),y_size_(y_size),is_master_(true){

    
    //descriptot 
}


bool Image::Load(const std::string& image_name_){
    FileHelper::IMAGE* im=nullptr;uint32_t w,h;
    std::tie(w,h,im)=FileHelper::LoadImage(image_name_);//this line should be modified
    if(im==nullptr)return false;
    auto image_size=4*w*h;
    x_size_=w,y_size_=h;
    device_->CreateGeneralBuffer(image_size,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buff_, staging_buff_mem_);
    void* data;
    vkMapMemory(device_->device_, staging_buff_mem_, 0, image_size, 0, &data);
    memcpy(data, im, image_size);
    vkUnmapMemory(device_->device_, staging_buff_mem_);
    FileHelper::FreeImage(im);
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(w);
    imageInfo.extent.height = static_cast<uint32_t>(h);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    assert(vkCreateImage(device_->device_, &imageInfo, nullptr, &image_) == VK_SUCCESS&&"failed to create image!");
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_->device_, image_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device_->FindMemoryType(memRequirements.memoryTypeBits,     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    assert(vkAllocateMemory(device_->device_, &allocInfo, nullptr, &image_mem_) == VK_SUCCESS && "failed to allocate image memory!");

    vkBindImageMemory(device_->device_, image_, image_mem_, 0);

    Transition(image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkCommandBuffer commandBuffer = commander_->BeginOneTimeCommand();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {w,h,1};
    vkCmdCopyBufferToImage(
        commandBuffer,
        staging_buff_,
        image_,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
    commander_->EndOneTimeCommand(std::move(commandBuffer));
    Transition(image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(device_->device_, staging_buff_, nullptr);
    vkFreeMemory(device_->device_, staging_buff_mem_, nullptr);
//Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image =image_;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    assert(vkCreateImageView(device_->device_, &viewInfo, nullptr, &image_view_) == VK_SUCCESS && "Create image view fails");
    descriptot_=new Descriptor();
    auto set_lay=descriptot_->CreateSetLayout(this,
    {
       // {0,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT},
        {1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT}
    }
    );
descriptot_->ContinueConstruct(
    {//these are descriptor info
        //{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,MAX_FRAMES_IN_FLIGHT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,commander_->MAX_FRAMES_IN_FLIGHT}
    }
);
return true;
} 

void Image::Transition(VkImage image, VkFormat format, VkImageLayout old,VkImageLayout new_){
    VkCommandBuffer command_buff = commander_->BeginOneTimeCommand();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout =old;
    barrier.newLayout = new_;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (old == VK_IMAGE_LAYOUT_UNDEFINED && new_ == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_ ==    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        assert(false && "unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buff,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    commander_->EndOneTimeCommand(std::move(command_buff));


}
VkDescriptorImageInfo Image::GetImageDescriptotInfo(int i){
    return VkDescriptorImageInfo{
        commander_->sampler_,
        image_view_,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
}
Image::~Image(){
    if(!is_master_)return;
    vkDestroyImageView(device_->device_,image_view_,nullptr);
    vkDestroyImage(device_->device_, image_, nullptr);
    vkFreeMemory(device_->device_, image_mem_, nullptr);
}

} // namespace Vulkan
