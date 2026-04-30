#include "Middleware.h"

void MiddlewarePipeline::use(const Middleware& middleware)
{
    _middlewares.push_back(middleware);
}

void MiddlewarePipeline::execute(const HttpRequest& request, const std::shared_ptr<Socket>& socket,
                                 const Next& final_handler) const
{
    std::function<void(size_t)> run = [&](size_t index)
    {
        if (index == _middlewares.size())
        {
            if (final_handler)
            {
                final_handler();
                return;
            }
        }

        Next next = [&run, index]() { run(index + 1); };
        _middlewares[index](request, socket, next);
    };

    run(0);
}
