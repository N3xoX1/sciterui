
export class App extends Element {

    nmatches = 0;

    render() {

        /*@Literal string to translate*/
        let littext = @"String to translate";

        /*@Form content*/
        return <main>
            <p @title="Title test" @>UI language</p>
            <hr/>
            {littext}
            <hr/>
            <p @>Tests: <span @>first test</span>,<span @>second test</span></p>
            <hr/>
            <label>Matches</label> : <input|integer(nmatches) min=0 max=100 step=1 value={this.nmatches} />
            <p @>{this.nmatches} matches</p>
        </main>;
    }

    ["on change at input(nmatches)"](evt,input) {
        this.componentUpdate { nmatches:input.value };
    }
}

