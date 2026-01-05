#include <simulation/PhysicsTransformBuffer.h>

namespace simulation {

void PhysicsTransformBuffer::writeTransform(entt::entity entity, float x, float y, float rotation)
{
    // Запись в буфер записи не требует блокировки,
    // так как главный поток работает только с буфером чтения
    (*m_writeBuffer)[entity] = BufferedTransform(x, y, rotation);
}

void PhysicsTransformBuffer::writeTransform(entt::entity entity, const core::TransformComponent& transform)
{
    (*m_writeBuffer)[entity] = BufferedTransform(transform);
}

void PhysicsTransformBuffer::swapBuffers()
{
    std::lock_guard<std::mutex> lock(m_swapMutex);

    // Меняем указатели местами
    std::swap(m_writeBuffer, m_readBuffer);

    // Очищаем новый буфер записи для следующего кадра
    m_writeBuffer->clear();
}

void PhysicsTransformBuffer::applyToRegistry(entt::registry& registry)
{
    // Буфер чтения не изменяется потоком физики после swap,
    // поэтому блокировка не нужна
    for (const auto& [entity, transform] : *m_readBuffer) {
        if (registry.valid(entity)) {
            auto* transformComp = registry.try_get<core::TransformComponent>(entity);
            if (transformComp) {
                transform.applyTo(*transformComp);
            }
        }
    }
}

void PhysicsTransformBuffer::clear()
{
    std::lock_guard<std::mutex> lock(m_swapMutex);
    m_bufferA.clear();
    m_bufferB.clear();
}

void PhysicsTransformBuffer::removeEntity(entt::entity entity)
{
    std::lock_guard<std::mutex> lock(m_swapMutex);
    m_bufferA.erase(entity);
    m_bufferB.erase(entity);
}

size_t PhysicsTransformBuffer::getWriteBufferSize() const
{
    return m_writeBuffer->size();
}

size_t PhysicsTransformBuffer::getReadBufferSize() const
{
    return m_readBuffer->size();
}

void PhysicsTransformBuffer::reserve(size_t capacity)
{
    m_bufferA.reserve(capacity);
    m_bufferB.reserve(capacity);
}

} // namespace simulation
