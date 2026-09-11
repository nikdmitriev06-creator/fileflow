#include "WorkerPool.h"

#include "application/processing/FileProcessor.h"

#include "infrastructure/logging/Logger.h"

namespace {

    class JobCompletionGuard {
    public:
        explicit JobCompletionGuard(
            fileflow::domain::JobQueue& queue
        )
            : queue_(queue)
        {
        }

        ~JobCompletionGuard()
        {
            // RAII гарантирует вызов taskCompleted()
            // при любом выходе из текущей области видимости.
            //
            // Это работает и при обычном завершении,
            // и при исключении.
            queue_.taskCompleted();
        }

        JobCompletionGuard(const JobCompletionGuard&) = delete;
        JobCompletionGuard& operator=(
            const JobCompletionGuard&
            ) = delete;

    private:
        fileflow::domain::JobQueue& queue_;
    };

} // namespace

namespace fileflow::domain {

    WorkerPool::WorkerPool(
        JobQueue& queue,
        std::size_t workerCount
    )
        : queue_(queue)
    {
        workers_.reserve(workerCount);

        for (std::size_t i = 0; i < workerCount; ++i) {

            workers_.emplace_back(
                [this, workerId = i](
                    std::stop_token /*stopToken*/
                    ) {
                        workerLoop(workerId);
                }
            );
        }
    }

    WorkerPool::~WorkerPool()
    {
        // Запрещаем добавление новых задач
        // и будим ожидающих worker'ов.
        queue_.shutdown();

        // std::jthread автоматически дождётся
        // завершения worker-потоков.
    }

    void WorkerPool::workerLoop(std::size_t workerId)
    {
        application::processing::FileProcessor processor;

        infrastructure::logging::Logger::info(
            "Worker " + std::to_string(workerId) + " started"
        );

        while (true) {

            // Если работы нет, worker блокируется внутри pop().
            auto job = queue_.pop();

            // nullptr означает:
            // shutdown активирован и новых задач больше нет.
            if (!job) {
                break;
            }

            // С этого момента задача считается
            // находящейся в состоянии processing.
            job->start();

            // Создаём RAII guard.
            //
            // Когда worker закончит текущую итерацию,
            // guard автоматически вызовет taskCompleted().
            JobCompletionGuard completionGuard(queue_);

            infrastructure::logging::Logger::info(
                "Worker " +
                std::to_string(workerId) +
                " processing job " +
                std::to_string(job->id()) +
                " (" +
                job->filename() +
                ")"
            );

            try {

                // Worker не знает деталей обработки.
                // Он просто передаёт Job специализированному компоненту.
                const auto result = processor.process(*job);

                job->complete(result.hash);

                infrastructure::logging::Logger::info(
                    "Worker " +
                    std::to_string(workerId) +
                    " completed job " +
                    std::to_string(job->id())
                );

            }
            catch (const std::exception& error) {

                // FileProcessor сообщил об ошибке.
                job->fail(error.what());

                infrastructure::logging::Logger::error(
                    "Worker " +
                    std::to_string(workerId) +
                    " failed job " +
                    std::to_string(job->id()) +
                    ": " +
                    error.what()
                );
            }
        }

        infrastructure::logging::Logger::info(
            "Worker " +
            std::to_string(workerId) +
            " stopped"
        );
    }

} // namespace fileflow::domain