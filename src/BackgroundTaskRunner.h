// // Created by August Pemberton on 17/05/2024.
//

#pragma once
#include <utility>
#include <juce_core/juce_core.h>
#include <choc/containers/choc_Value.h>

namespace imagiro {
    class BackgroundTaskRunner : juce::Thread {
    public:

        struct Listener {
            virtual void OnTaskFinished(int taskID, const choc::value::ValueView& result) {}
        };
        void addListener(Listener* l) { listeners.add(l); }
        void removeListener(Listener* l) { listeners.remove(l); }

        struct Task {
            std::function<choc::value::Value()> fn {};
            int id = -1;
        };

        BackgroundTaskRunner() : juce::Thread("Background Task Runner") {
            startThread();
        }

        ~BackgroundTaskRunner() override {
            stopThread(500);
        }

        void clearTasks() {
            clearTasksFlag = true;
        }

        void run() override {
            while (!threadShouldExit()) {
                while (tasks.try_dequeue(temp)) {
                    if (threadShouldExit()) return;

                    // if just cleared, ignore the rest of the task
                    if (clearTasksFlag) continue;

                    processTask(temp);
                }

                clearTasksFlag = false;
                juce::Thread::wait(-1);
            }
        }

        void processTask(const Task &task) {
            auto result = task.fn();
            listeners.call(&Listener::OnTaskFinished, task.id, result);
        }

        int queueTask(Task task) {
            task.id = tasksQueued++;
            tasks.enqueue(task);
            notify();
            return task.id;
        }

    private:
        juce::ListenerList<Listener> listeners {};
        Task temp;
        moodycamel::ReaderWriterQueue<Task> tasks{512};

        int tasksQueued = 0;
        std::atomic<bool> clearTasksFlag{false};
    };
}