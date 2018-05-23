/*
	Systems for performing asynchronous tasks and waiting for them, etc.
*/
#pragma once
#include <memory>
#include "Shared/Unique.hpp"
#include "Shared/Delegate.hpp"
#include "Thread.hpp"
#include "Timer.hpp"
#include "Vector.hpp"
#include "List.hpp"

/*
	Additional job flags,
	this allows clustering IO operation onto the same thread as to not lock up the system
*/
enum class JobFlags : uint8
{
	None = 0,
	IO = 0x1,
};

JobFlags operator|(JobFlags a, JobFlags b);
JobFlags operator&(JobFlags a, JobFlags b);

/*
	A single task that gets completed by the JobSheduler
	abstract
	override this class or use a LambdaJob to create a runnable task
*/
class JobBase : public Unique
{
public:
	virtual ~JobBase() = default;

	bool IsFinished() const;
	bool IsSuccessfull() const;
	bool IsQueued() const;

	// Either cancel this job or wait till it finished if it is already being processed
	void Terminate();

	// Flags for jobs
	// make sure to add the IO flag if this job performs file operations
	JobFlags jobFlags = JobFlags::None;

	// Performs the task to be done, returns success
	virtual bool Run() = 0;
	// Called on the main thread before the delegate callback
	virtual void Finalize();

	// Called when finished
	//	called from main thread when JobSheduler::Update is called
	Delegate<shared_ptr<JobBase>> OnFinished;

	// Create job from lambda function
	template <typename Lambda, typename... Args>
	static shared_ptr<JobBase> CreateLambda(Lambda&& obj, Args ...);

private:
	bool m_ret = false;
	bool m_finished = false;
	class JobSheduler* m_sheduler = nullptr;
	friend class JobSheduler;
};

/*
	Job that runs a lambda that returns a boolean
*/
template <typename Lambda, typename... Args>
class LambdaJob : public JobBase
{
public:
	LambdaJob(Lambda& lambda, Args ... args)
		: m_lambda(lambda)
	{
		m_args = {args...};
	}

	virtual bool Run()
	{
		return m_CallFunc(std::index_sequence_for<Args...>{});
	}

private:
	template <size_t... I>
	bool m_CallFunc(std::index_sequence<I...>)
	{
		return m_lambda(std::get<I>(m_args)...);
	}

	std::tuple<Args...> m_args;
	Lambda m_lambda;
};

typedef std::shared_ptr<JobBase> Job;

template <typename Lambda, typename... Args>
Job JobBase::CreateLambda(Lambda&& obj, Args ... args)
{
	return shared_ptr<JobBase>(new LambdaJob<Lambda, Args...>(obj, args...));
}

struct JobThread
{
	// Thread index
	uint32 index = 0;
	bool terminate = false;
	Thread thread;

	// Job currently being processed
	Job activeJob;

	Timer idleDuration;

	// Terminate this thread
	void Terminate()
	{
		if (terminate)
			return; // Already terminated
		terminate = true;
		if (thread.joinable())
			thread.join();
	}

	bool IsActive() const
	{
		return activeJob != nullptr;
	}
};

/*
	The manager for performing asynchronous tasks
	you should only have one of these
*/
class JobSheduler : public Unique
{
public:
	JobSheduler();
	~JobSheduler();
	void Update();

	// Queue job
	bool Queue(Job job);

private:
	// Contains tasks to be done
	List<Job> m_jobQueue;
	// Contains tasks that are done
	List<Job> m_finishedJobs;

	friend class JobBase;

	Mutex m_lock;
	Vector<JobThread*> m_threadPool;

	void m_JobThread(JobThread* myThread);
	bool QueueUnchecked(Job job);
};
